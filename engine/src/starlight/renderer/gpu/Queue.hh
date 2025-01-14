#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/utils/Enum.hh"

#include "CommandBuffer.hh"
#include "Sync.hh"

namespace sl {

struct Queue : NonCopyable, NonMovable {
    enum class Type : u8 {
        none     = 0x0,
        graphics = 0x1,
        present  = 0x2,
        transfer = 0x4,
        compute  = 0x8
    };

    virtual ~Queue() = default;

    virtual void submit(
      v2::CommandBuffer& commandBuffer, Semaphore* waitSemaphore = nullptr,
      Semaphore* signalSemaphore = nullptr, Fence* fence = nullptr
    ) = 0;
};

constexpr void enableBitOperations(Queue::Type);

}  // namespace sl
