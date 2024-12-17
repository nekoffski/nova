#pragma once

#include <starlight/scene/ecs/Entity.hh>

namespace sle::events {

struct EntitySelected {
    sl::Entity* entity;
};

struct SceneSerialization {
    enum class Action { serialize, deserialize };

    Action action;
    std::string path;
};

}  // namespace sle::events
