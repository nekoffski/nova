#pragma once

#include <functional>

#include <starlight/scene/ecs/Entity.hh>

namespace sle::events {

struct EntitySelected {
    sl::Entity* entity;
    bool clearComponentCallback = false;
};

struct SetComponentUICallback {
    using Callback = std::function<void()>;
    Callback callback;
};

struct SetResourceUICallback {
    using Callback = std::function<void()>;
    Callback callback;
};

struct SceneSerialization {
    enum class Action { serialize, deserialize };

    Action action;
    std::string path;
};

}  // namespace sle::events
