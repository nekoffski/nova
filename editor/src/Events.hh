#pragma once

#include <starlight/scene/ecs/Entity.hh>

namespace sle::events {

struct EntitySelected {
    sl::Entity* entity;
};

}  // namespace sle::events
