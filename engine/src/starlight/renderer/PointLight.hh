#pragma once

#include "starlight/core/math/Core.h"

namespace sl {

struct PointLight {
    Vec4<f32> position           = Vec4<f32>{ 0.0f, 0.0f, 0.0, 1.0f };
    Vec4<f32> color              = Vec4<f32>{ 1.0f };
    Vec4<f32> attenuationFactors = Vec4<f32>{ 1.0f, 0.2f, 0.33f, 0.0f };
};

}  // namespace sl
