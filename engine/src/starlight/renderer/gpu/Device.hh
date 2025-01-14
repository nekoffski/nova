#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/Context.hh"
#include "starlight/core/memory/Memory.hh"

#include "Queue.hh"

namespace sl {

class Device : public NonCopyable, public NonMovable {
public:
    virtual ~Device() = default;

    static OwningPtr<Device> create(Context& context);

    virtual void waitIdle()                   = 0;
    virtual Queue& getQueue(Queue::Type type) = 0;
};

}  // namespace sl
