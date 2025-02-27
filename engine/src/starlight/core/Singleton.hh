#pragma once

#include "Log.hh"
#include "Core.hh"

namespace sl {

template <typename T> class Singleton : public NonMovable, public NonCopyable {
public:
    explicit Singleton() {
        log::expect(not isCreated(), "Only 1 instance of singleton is allowed");
        s_instance = static_cast<T*>(this);
    }

    virtual ~Singleton() { s_instance = nullptr; }

    static T& get() {
        log::expect(isCreated(), "Singleton instance has not been created");
        return *s_instance;
    }

    static T* getPtr() { return s_instance; }

    static bool isCreated() { return s_instance != nullptr; }

private:
    inline static T* s_instance = nullptr;
};

}  // namespace sl
