#pragma once

#include "Log.hh"
#include "Core.hh"
#include "Utils.hh"

namespace sl {

template <typename T> class Singleton : public NonMovable, public NonCopyable {
    inline static std::string className = getTypeName<T>();

public:
    explicit Singleton() {
        log::expect(
          not isCreated(), "Only 1 instance of singleton ({}) is allowed", className
        );
        s_instance = static_cast<T*>(this);
    }

    virtual ~Singleton() { s_instance = nullptr; }

    static T& get() {
        log::expect(
          isCreated(), "Singleton ({}) instance has not been created", className
        );
        return *s_instance;
    }

    static T* getPtr() { return s_instance; }

    static bool isCreated() { return s_instance != nullptr; }

private:
    inline static T* s_instance = nullptr;
};

namespace detail {

template <typename T>
requires std::derived_from<T, Singleton<T>>
void expectCreatedImpl() {
    log::expect(
      T::isCreated(), "{} is required to be created at that time", getTypeName<T>()
    );
}

}  // namespace detail

template <typename... T>
requires(std::derived_from<T, Singleton<T>> && ...)
void expectCreated() {
    (detail::expectCreatedImpl<T>(), ...);
}

}  // namespace sl
