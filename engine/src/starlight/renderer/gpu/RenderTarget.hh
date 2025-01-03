#pragma once

#include <vector>

#include "starlight/core/Core.hh"
#include "starlight/core/math/Core.hh"
#include "Texture.hh"

namespace sl {

struct RenderTarget {
    Vec2<u32> size;
    Texture* colorAttachment = nullptr;
    Texture* depthAttachment = nullptr;
};

}  // namespace sl
