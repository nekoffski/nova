#pragma once

#include <optional>

#include "starlight/core/Core.hh"
#include "starlight/core/memory/UniquePtr.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/window/Window.hh"

#include "Sync.hh"
#include "Texture.hh"

namespace sl {

class Swapchain {
public:
    static UniquePtr<Swapchain> create(
      const Vec2<u32>& size = Window::get().getFramebufferSize()
    );

    virtual ~Swapchain() = default;

    virtual void recreate(const Vec2<u32>& size) = 0;

    virtual std::optional<u32> acquireNextImageIndex(
      Semaphore* imageSemaphore = nullptr, Fence* fence = nullptr,
      Nanoseconds timeout = max<u64>()
    )                                 = 0;
    virtual u32 getImageCount() const = 0;

    virtual Texture* getImage(u32 index) = 0;
    virtual Texture* getDepthBuffer()    = 0;
};

}  // namespace sl
