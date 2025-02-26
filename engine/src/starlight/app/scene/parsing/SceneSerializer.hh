#pragma once

#include <unordered_map>
#include "starlight/core/Singleton.hh"

#include "starlight/core/FileSystem.hh"
#include "starlight/app/scene/Scene.hh"
#include "starlight/core/Json.hh"

#include "ComponentSerializer.hh"

namespace sl {

class SceneSerializer {
    using Serializers =
      std::unordered_map<std::type_index, UniquePtr<ComponentSerializerBase>>;

public:
    void serialize(Scene& scene, const std::string& path, const FileSystem& fs);

    template <typename Serializer>
    requires std::is_base_of_v<ComponentSerializerBase, Serializer>
    SceneSerializer& addSerializer() {
        auto serializer     = UniquePtr<Serializer>::create();
        const auto type     = serializer->getTypeIndex();
        m_serializers[type] = std::move(serializer);

        return *this;
    }

private:
    nlohmann::json parseEntity(Entity& entity);

    Serializers m_serializers;
};

}  // namespace sl
