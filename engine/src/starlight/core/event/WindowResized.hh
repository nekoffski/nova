#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/Utils.hh"

namespace sl {

struct WindowResized {
    Vec2<u32> size;
};

inline std::string toString(const WindowResized& event) {
    return fmt::format("WindowResized[{}/{}]", event.size.w, event.size.h);
}

}  // namespace sl
