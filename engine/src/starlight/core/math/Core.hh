#pragma once

#include <numbers>

#include "starlight/core/Core.hh"

#include "Glm.hh"
#include "Vec.hh"
#include "Rect.hh"
#include "Extent.hh"

namespace sl {

const Mat4<f32> identityMatrix = Mat4<f32>{ 1.0f };
const float pi                 = std::numbers::pi_v<float>;

}  // namespace sl
