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

template <typename T> constexpr u64 getSize() { return 0u; }

template <> constexpr u64 getSize<Vec2<f32>>() { return 2u; }
template <> constexpr u64 getSize<Vec3<f32>>() { return 3u; }
template <> constexpr u64 getSize<Vec4<f32>>() { return 4u; }

template <> constexpr u64 getSize<Mat2<f32>>() { return 4u; }
template <> constexpr u64 getSize<Mat3<f32>>() { return 9u; }
template <> constexpr u64 getSize<Mat4<f32>>() { return 16u; }

}  // namespace sl
