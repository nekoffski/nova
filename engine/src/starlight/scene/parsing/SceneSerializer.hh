#pragma once

#include <unordered_map>
#include <kc/core/Singleton.hpp>

#include "starlight/core/FileSystem.hh"
#include "starlight/scene/Scene.hh"
#include "starlight/core/utils/Json.hh"

#include "ComponentSerializer.hh"

namespace sl {

class SceneSerializer {
    using Serializers =
      std::unordered_map<std::type_index, OwningPtr<ComponentSerializerBase>>;

public:
    void serialize(Scene& scene, const std::string& path, const FileSystem& fs);

    template <typename Serializer>
    requires std::is_base_of_v<ComponentSerializerBase, Serializer>
    SceneSerializer& addSerializer() {
        auto serializer     = createOwningPtr<Serializer>();
        const auto type     = serializer->getTypeIndex();
        m_serializers[type] = std::move(serializer);

        return *this;
    }

private:
    kc::json::Node parseEntity(Entity& entity);

    Serializers m_serializers;
};

}  // namespace sl
