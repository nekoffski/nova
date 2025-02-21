#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/Singleton.hh"
#include "starlight/core/memory/Memory.hh"

#include "fwd.hh"
#include "Queue.hh"

namespace sl {

class Device : public Singleton<Device> {
public:
    struct Impl : NonCopyable, NonMovable {
        virtual ~Impl() = default;

        virtual void waitIdle()                   = 0;
        virtual Queue& getQueue(Queue::Type type) = 0;

        static UniquePointer<Impl> create();
    };

    explicit Device();

    void waitIdle();
    Queue& getQueue(Queue::Type type);

    Queue& getGraphicsQueue();
    Queue& getPresentQueue();

    Impl& getImpl();

private:
    UniquePointer<Impl> m_impl;
};

}  // namespace sl
