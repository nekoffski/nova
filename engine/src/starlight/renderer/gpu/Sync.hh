#pragma once

#include <limits>

#include "starlight/core/Core.hh"

namespace sl {

class Fence : public NonCopyable, public NonMovable {
public:
    enum class State : u8 { signaled, notSignaled };

    explicit Fence(State state);
    virtual ~Fence() = default;

    virtual bool wait(Nanoseconds timeout = u64Max) = 0;
    virtual void reset()                            = 0;

private:
    State m_state;
};

struct Semaphore : public NonCopyable, public NonMovable {
    virtual ~Semaphore() = default;
};

}  // namespace sl
