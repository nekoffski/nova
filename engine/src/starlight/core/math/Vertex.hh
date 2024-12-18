#pragma once

#include <kc/core/String.h>

#include "Core.hh"

namespace sl {

struct Vertex2 {
    Vec2<f32> position;
    Vec2<f32> textureCoordinates;
};

struct Vertex3 {
    Vec3<f32> position;
    Vec3<f32> normal;
    Vec2<f32> textureCoordinates;
    Vec4<f32> color;
    Vec4<f32> tangent;
};

}  // namespace sl

DEFINE_TO_STRING(sl::Vertex3, v) {
    return fmt::format(
      "Vertex3: position={}, texture coordinates={}, normal={}, color={}, tangent={}",
      v.position, v.textureCoordinates, v.normal, v.color, v.tangent
    );
}
