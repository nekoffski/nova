#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/utils/Enum.hh"

#include "Commands.hh"

namespace sl {

namespace v2 {

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

constexpr void enableBitOperations(CommandBuffer::BeginFlags);

}  // namespace v2

struct CommandBuffer : public NonCopyable, public NonMovable {
    enum class State : unsigned char {
        ready,
        recording,
        inRenderPass,
        recordingEnded,
        submitted,
        notAllocated
    };

    enum class Severity : unsigned char { primary, nonPrimary };

    struct BeginFlags {
        bool isSingleUse;
        bool isRenderpassContinue;
        bool isSimultaneousUse;
    };

    virtual void begin(const BeginFlags&) = 0;
    virtual void end()                    = 0;
};

}  // namespace sl
