#pragma once

#include "starlight/core/Core.hh"
#include "CommandBuffer.hh"

namespace sl {

struct Pipeline : public NonCopyable, public NonMovable {
    virtual ~Pipeline() = default;

    virtual void bind(CommandBuffer& commandBuffer) = 0;
};

}  // namespace sl
