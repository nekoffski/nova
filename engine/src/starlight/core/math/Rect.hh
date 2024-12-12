#pragma once

#include "starlight/core/Core.hh"
#include "Glm.hh"

namespace sl {

template <typename T> struct Rect2 {
    Vec2<T> offset;
    Vec2<T> size;
};

template <typename T> struct Rect3 {
    Vec3<T> offset;
    Vec3<T> size;
};

}  // namespace sl
