#pragma once

#include <unordered_map>
#include "starlight/core/Singleton.hh"

#include "starlight/core/FileSystem.hh"
#include "starlight/app/scene/Scene.hh"
#include "starlight/core/Json.hh"

#include "ComponentDeserializer.hh"

namespace sl {

class SceneDeserializer {
    using Deserializers =
      std::unordered_map<std::string, UniquePtr<ComponentDeserializer>>;

public:
    void deserialize(Scene& scene, const std::string& path, const FileSystem& fs);

    template <typename Deserializer>
    requires std::is_base_of_v<ComponentDeserializer, Deserializer>
    SceneDeserializer& addDeserializer() {
        auto deserializer     = UniquePtr<Deserializer>::create();
        const auto name       = deserializer->getName();
        m_deserializers[name] = std::move(deserializer);

        return *this;
    }

private:
    void parseEntity(Scene& scene, const nlohmann::json& node);

    Deserializers m_deserializers;
};

}  // namespace sl
