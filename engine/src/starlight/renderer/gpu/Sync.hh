#pragma once

#include "starlight/core/memory/Memory.hh"
#include "starlight/core/Core.hh"

#include "fwd.hh"

namespace sl {

struct Fence : public NonCopyable, public NonMovable {
    enum class State : u8 { signaled, notSignaled };

    static UniquePointer<Fence> create(State state);

    virtual ~Fence() = default;

    virtual bool wait(Nanoseconds timeout = max<u64>()) = 0;
    virtual void reset()                                = 0;
};

struct Semaphore : public NonCopyable, public NonMovable {
    static UniquePointer<Semaphore> create();

    virtual ~Semaphore() = default;
};

}  // namespace sl
