#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <memory>
#include <functional>
#include <concepts>
#include <span>
#include <string_view>

#include "starlight/core/math/Core.hh"
#include "starlight/core/memory/Memory.hh"
#include "starlight/core/Id.hh"
#include "starlight/core/FileSystem.hh"
#include "starlight/core/Core.hh"
#include "starlight/core/Log.hh"
#include "starlight/core/Resource.hh"

#include "starlight/renderer/fwd.hh"
#include "Texture.hh"

namespace sl {

template <typename T>
concept GlmCompatible = requires(T object) {
    { glm::value_ptr(object) } -> std::convertible_to<void*>;
};

class Shader : public NonMovable, public Identificable<Shader> {
public:
    enum class Scope : u8 {
        global   = 0,  // updated once per frame
        instance = 1,  // per instance
        local    = 2   // per object
    };

    static Scope scopeFromString(const std::string& name);
    static std::string scopeToString(Scope scope);

    struct Attribute {
        enum class Type : u8 {
            float32,
            float32_2,
            float32_3,
            float32_4,
            mat4,
            int8,
            uint8,
            int16,
            uint16,
            int32,
            uint32
        };

        static Type typeFromString(const std::string& name);
        static std::string typeToString(Type type);
        static u32 getTypeSize(Type type);

        std::string name;
        Type type;
        u32 size;
    };

    struct Stage {
        enum class Type : u8 { vertex, geometry, fragment, compute };

        static Type typeFromString(const std::string& name);
        static std::string typeToString(Type stage);

        Type type;
        std::string source;
    };

    struct Uniform {
        enum class Type : u8 {
            float32,
            float32_2,
            float32_3,
            float32_4,
            int8,
            uint8,
            int16,
            uint16,
            int32,
            uint32,
            mat4,
            sampler,
            custom
        };

        struct Properties {
            std::string name;
            u8 size;
            u32 location;
            Uniform::Type type;
            Scope scope;
        };

        bool isSampler() const;

        static Type typeFromString(const std::string& name);
        static std::string typeToString(Type type);
        static u32 getTypeSize(Type type);

        u64 offset;
        u16 location;
        u16 index;
        u16 size;
        Id8 setIndex;

        Scope scope;
        Type type;
    };

    struct Properties {
        bool useInstances;
        bool useLocals;

        std::vector<Attribute> attributes;
        std::vector<Stage> stages;
        std::vector<Uniform::Properties> uniformProperties;
        Texture* defaultTexture;

        CullMode cullMode;
        PolygonMode polygonMode;
    };

    class UniformProxy {
        friend class Shader;

    public:
        template <typename T>
        requires GlmCompatible<T>
        void set(const std::string& uniform, const T& value) {
            m_shader.setUniform(uniform, glm::value_ptr(value), m_commandBuffer);
        }

        template <typename T>
        requires(not std::is_pointer_v<T> && not GlmCompatible<T>)
        void set(const std::string& uniform, const T& value) {
            m_shader.setUniform(uniform, std::addressof(value), m_commandBuffer);
        }

        template <typename T>
        void set(const std::string& uniform, const std::vector<T>& array) {
            m_shader.setUniform(uniform, array.data(), m_commandBuffer);
        }

        template <typename T> void set(const std::string& uniform, const T* value) {
            m_shader.setUniform(uniform, value, m_commandBuffer);
        }

        void set(const std::string& uniform, const Texture* value);

    private:
        explicit UniformProxy(Shader& shader, CommandBuffer& commandBuffer);
        Shader& m_shader;
        CommandBuffer& m_commandBuffer;
    };

    class Instance : NonMovable, NonCopyable {
        friend class Shader;

        struct Record {
            u64 instanceId;
            ResourceRef<Shader> shader;
        };

    public:
        explicit Instance(const std::vector<Texture*>& textures);
        ~Instance();

        u32 getId(ResourceRef<Shader> shader);

    private:
        std::vector<Texture*> m_textures;
        std::unordered_map<u64, Record> m_records;
    };

    using UniformCallback = std::function<void(UniformProxy&)>;

    virtual ~Shader() = default;

    // FIXME: temporary till shader/pipeline refactor
    virtual void bindPipeline(Pipeline& pipeline) = 0;

    virtual void use(CommandBuffer&) = 0;

    void setGlobalUniforms(
      CommandBuffer& commandBuffer, u32 imageIndex, UniformCallback&& callback
    );
    void setInstanceUniforms(
      CommandBuffer& commandBuffer, u32 instanceId, u32 imageIndex,
      UniformCallback&& callback
    );
    void setLocalUniforms(CommandBuffer& commandBuffer, UniformCallback&& callback);

protected:
    explicit Shader(const Properties& props);

private:
    virtual u32 acquireInstanceResources(const std::vector<Texture*>& textures) = 0;
    virtual void releaseInstanceResources(u32 instanceId)                       = 0;

    virtual void bindGlobals()                                               = 0;
    virtual void bindInstance(u32 instanceId)                                = 0;
    virtual void applyGlobals(CommandBuffer& commandBuffer, u32 imageIndex)  = 0;
    virtual void applyInstance(CommandBuffer& commandBuffer, u32 imageIndex) = 0;

    virtual void setSampler(const std::string& uniform, const Texture* value) = 0;
    virtual void setUniform(
      const std::string& name, const void* value, CommandBuffer& commandBuffer
    ) = 0;

protected:
    std::string m_name;
    bool m_useInstances;
    bool m_useLocals;

    CullMode m_cullMode;
    PolygonMode m_polygonMode;
};

}  // namespace sl
