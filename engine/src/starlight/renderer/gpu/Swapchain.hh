#pragma once

#include <optional>

#include "starlight/core/Core.hh"

#include "Sync.hh"

namespace sl {

class Swapchain {
public:
    virtual ~Swapchain() = default;

    virtual std::optional<u8> acquireNextImageIndex(
      Semaphore* imageSemaphore = nullptr, Fence* fence = nullptr,
      Nanoseconds timeout = u64Max
    )                                = 0;
    virtual u8 getImageCount() const = 0;
};

}  // namespace sl
