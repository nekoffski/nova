#pragma once

#include <string>
#include <span>

#include <kc/core/Meta.hpp>

#include "fwd.hh"

#include "starlight/core/math/Extent.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/core/math/Vertex.hh"
#include "starlight/core/utils/Resource.hh"

namespace sl {
namespace detail {

template <typename T>
concept ExtentType = kc::core::is_one_of2_v<T, Extent2, Extent3>;

template <typename T>
concept VertexType = kc::core::is_one_of2_v<T, Vertex2, Vertex3>;

template <ExtentType Extent, VertexType Vertex>
Extent calculateExtent(const std::vector<Vertex>& vertices) {
    ASSERT(vertices.size() > 0, "vertices.size() == 0");
    using Vector = std::remove_cv_t<typeof(vertices[0].position)>;

    Vector min(0.0f);
    Vector max(0.0f);

    constexpr auto components = []() {
        if constexpr (std::is_same_v<Vertex, Vertex2>)
            return 2u;
        else
            return 3u;
    }();

    for (const auto& vertex : vertices) {
        for (u8 i = 0; i < components; ++i) {
            const auto& component = vertex.position[i];
            if (component < min[i]) min[i] = component;
            if (component > max[i]) max[i] = component;
        }
    }

    return Extent(min, max);
}

}  // namespace detail

struct PlaneProperties {
    float width;
    float height;
    u32 xSegments;
    u32 zSegments;
};

struct CubeProperties {
    float width;
    float height;
    float depth;
    u32 xTile;
    u32 yTile;
};

struct SphereProperties {
    u32 stacks;
    u32 slices;
    float radius;
};

class Mesh : public NonMovable, public Identificable<Mesh> {
public:
    struct Data {
        u64 vertexSize;
        u64 vertexCount;
        const void* vertexData;
        std::span<const u32> indices;
        Extent3 extent;
    };

    template <detail::VertexType Vertex, detail::ExtentType Extent>
    struct Properties {
        std::vector<u32> indices;
        std::vector<Vertex> vertices;

        Extent calculateExtent() const {
            return detail::calculateExtent<Extent, Vertex>(vertices);
        }

        Data toMeshData() const& {
            return Data(
              sizeof(Vertex), vertices.size(), vertices.data(), indices,
              calculateExtent()
            );
        }
    };

    struct Properties3D final : public Properties<Vertex3, Extent3> {
        explicit Properties3D() = default;
        explicit Properties3D(const PlaneProperties& props);
        explicit Properties3D(const CubeProperties& props);
        explicit Properties3D(const SphereProperties& props);

        void generateTangents();
        void generateNormals();
    };

    struct Properties2D final : public Properties<Vertex2, Extent2> {};

    struct BufferDescription {
        u64 vertexCount;
        u64 vertexElementSize;
        u64 vertexBufferOffset;
        u64 indexCount;
        u64 indexElementSize;
        u64 indexBufferOffset;

        u64 indicesTotalSize() const { return indexCount * indexElementSize; }
        u64 verticesTotalSize() const { return vertexCount * vertexElementSize; }
    };

    virtual ~Mesh() = default;

    static ResourceRef<Mesh> load(
      const Properties2D& config, const std::string& name
    );
    static ResourceRef<Mesh> load(
      const Properties3D& config, const std::string& name
    );

    template <typename T>
    requires std::is_constructible_v<Properties3D, const T&>
    static ResourceRef<Mesh> load(const T& properties, const std::string& name) {
        return load(Properties3D{ properties }, name);
    }

    static ResourceRef<Mesh> find(const std::string& name);
    static ResourceRef<Mesh> getCube();
    static ResourceRef<Mesh> getUnitSphere();
    static ResourceRef<Mesh> getPlane();

    const BufferDescription& getDataDescription() const;
    const Extent3& getExtent() const;

protected:
    explicit Mesh(const Data& data);

    BufferDescription m_dataDescription;
    Extent3 m_extent;
};

class MeshManager
    : public ResourceManager<Mesh>,
      public kc::core::Singleton<MeshManager> {
public:
    explicit MeshManager(RendererBackend& renderer);

    ResourceRef<Mesh> load(
      const Mesh::Properties2D& config, const std::string& name
    );
    ResourceRef<Mesh> load(
      const Mesh::Properties3D& config, const std::string& name
    );

    ResourceRef<Mesh> getCube();
    ResourceRef<Mesh> getUnitSphere();
    ResourceRef<Mesh> getPlane();

private:
    void createDefaults();

    OwningPtr<Mesh> m_cube;
    OwningPtr<Mesh> m_unitSphere;
    OwningPtr<Mesh> m_plane;

    RendererBackend& m_renderer;
};

}  // namespace sl
