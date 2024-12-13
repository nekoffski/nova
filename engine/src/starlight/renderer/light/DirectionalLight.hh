#pragma once

#include <starlight/core/math/Core.hh>

namespace sl {

struct DirectionalLight {
    alignas(16) Vec4<f32> color     = { 0.8f, 0.8f, 0.8f, 1.0f };
    alignas(16) Vec3<f32> direction = { -0.57735f, -0.57735f, -0.57735f };

    std::string toString() const;
};

}  // namespace sl