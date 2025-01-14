#pragma once

#include <optional>

#include "starlight/core/Core.hh"

#include "Sync.hh"
#include "Texture.hh"

namespace sl {

class Swapchain {
public:
    virtual ~Swapchain() = default;

    virtual void recreate(const Vec2<u32>& size) = 0;

    virtual std::optional<u8> acquireNextImageIndex(
      Semaphore* imageSemaphore = nullptr, Fence* fence = nullptr,
      Nanoseconds timeout = u64Max
    )                                = 0;
    virtual u8 getImageCount() const = 0;

    virtual Texture* getImage(u8 index) = 0;
    virtual Texture* getDepthBuffer()   = 0;
};

}  // namespace sl
