#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/memory/Memory.hh"
#include "starlight/core/Enum.hh"

#include "fwd.hh"

#include "Commands.hh"

namespace sl {

struct CommandBuffer : public NonCopyable, public NonMovable {
    class Immediate : public NonCopyable, public NonMovable {
    public:
        Immediate(Queue& queue);
        ~Immediate();

        CommandBuffer& get();
        operator CommandBuffer&();

    private:
        UniquePointer<CommandBuffer> m_commandBuffer;
        Queue& m_queue;
    };

    enum class Severity : unsigned char { primary, nonPrimary };

    enum class BeginFlags : u8 {
        none                 = 0x0,
        singleUse            = 0x1,
        isRenderpassContinue = 0x2,
        simultaneousUse      = 0x4
    };

    static UniquePointer<CommandBuffer> create(
      Severity severity = Severity::primary
    );

    virtual ~CommandBuffer() = default;

    virtual void begin(BeginFlags beginFlags = BeginFlags::none) = 0;
    virtual void end()                                           = 0;
    virtual void execute(const Command& command)                 = 0;
};

constexpr void enableBitOperations(CommandBuffer::BeginFlags);

}  // namespace sl
