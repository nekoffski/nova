#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/utils/Enum.hh"

#include "CommandBuffer.hh"
#include "Sync.hh"
#include "Swapchain.hh"

namespace sl {

struct Queue : NonCopyable, NonMovable {
    enum class Type : u8 {
        none     = 0x0,
        graphics = 0x1,
        present  = 0x2,
        transfer = 0x4,
        compute  = 0x8
    };

    struct SubmitInfo {
        v2::CommandBuffer& commandBuffer;
        Semaphore* waitSemaphore   = nullptr;
        Semaphore* signalSemaphore = nullptr;
        Fence* fence               = nullptr;
    };

    struct PresentInfo {
        Swapchain& swapchain;
        u8 imageIndex;
        Semaphore* waitSemaphore = nullptr;
    };

    virtual ~Queue() = default;

    virtual bool submit(const SubmitInfo& submitInfo)    = 0;
    virtual bool present(const PresentInfo& presentInfo) = 0;
};

constexpr void enableBitOperations(Queue::Type);

}  // namespace sl
