#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/memory/Memory.hh"
#include "starlight/core/utils/Enum.hh"

#include "fwd.hh"

#include "Commands.hh"

namespace sl {

struct CommandBuffer : public NonCopyable, public NonMovable {
    enum class Severity : unsigned char { primary, nonPrimary };

    enum class BeginFlags : u8 {
        none                 = 0x0,
        singleUse            = 0x1,
        isRenderpassContinue = 0x2,
        simultaneousUse      = 0x4
    };

    virtual void begin(BeginFlags beginFlags = BeginFlags::none) = 0;
    virtual void end()                                           = 0;
    virtual void execute(const Command& command)                 = 0;
};

class ImmediateCommandBuffer : public NonCopyable, public NonMovable {
public:
    ImmediateCommandBuffer(OwningPtr<CommandBuffer> commandBuffer, Queue& queue);
    ~ImmediateCommandBuffer();

    CommandBuffer& get();
    operator CommandBuffer&();

private:
    OwningPtr<CommandBuffer> m_commandBuffer;
    Queue& m_queue;
};

constexpr void enableBitOperations(CommandBuffer::BeginFlags);

}  // namespace sl
