#pragma once

#include <variant>

#include "starlight/core/Core.hh"
#include "starlight/core/math/Core.hh"

namespace sl {

struct SetViewportCommand {
    Vec2<u32> offset;
    Vec2<u32> size;
};

struct SetScissorsCommand {
    Vec2<u32> offset;
    Vec2<u32> size;
};

using Command = std::variant<SetViewportCommand, SetScissorsCommand>;

}  // namespace sl
