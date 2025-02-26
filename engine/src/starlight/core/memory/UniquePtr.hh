#pragma once

#include <concepts>
#include <utility>

#include "starlight/core/Core.hh"

#include "Allocator.hh"
#include "Mallocator.hh"

namespace sl {

template <typename T> class UniquePtr : public NonCopyable {
    template <typename C> friend class UniquePtr;

    struct PrivateConstructorTag {};

public:
    explicit UniquePtr() : m_allocator(nullptr), m_buffer(nullptr) {}
    UniquePtr(std::nullptr_t) : UniquePtr() {}

    ~UniquePtr() noexcept { clear(); }

    void clear() {
        if (m_buffer) {
            m_buffer->~T();
            m_allocator->deallocate(m_buffer, 1);
            m_buffer = nullptr;
        }
    }

    UniquePtr& operator=(UniquePtr&& oth) {
        clear();

        m_buffer    = std::exchange(oth.m_buffer, nullptr);
        m_allocator = std::exchange(oth.m_allocator, nullptr);

        return *this;
    }

    template <typename F>
    requires std::derived_from<F, T>
    UniquePtr(UniquePtr<F>&& oth) {
        m_buffer    = std::exchange(oth.m_buffer, nullptr);
        m_allocator = std::exchange(oth.m_allocator, nullptr);
    }

    UniquePtr(UniquePtr&& oth) {
        m_buffer    = std::exchange(oth.m_buffer, nullptr);
        m_allocator = std::exchange(oth.m_allocator, nullptr);
    }

    T& operator*() { return *m_buffer; }
    const T& operator*() const { return *m_buffer; }

    T* operator->() { return m_buffer; }
    const T* operator->() const { return m_buffer; }

    T* get() { return m_buffer; }
    const T* get() const { return m_buffer; }

    Allocator* getAllocator() { return m_allocator; }

    operator bool() const { return m_buffer != nullptr; }

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    static UniquePtr<T> create(Allocator* allocator, Args&&... args) {
        return UniquePtr<T>{
            PrivateConstructorTag{}, allocator, std::forward<Args>(args)...
        };
    }

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    static UniquePtr<T> create(Args&&... args) {
        return UniquePtr<T>{
            PrivateConstructorTag{}, &s_defaultAllocator, std::forward<Args>(args)...
        };
    }

private:
    template <typename... Args>
    requires std::constructible_from<T, Args...>
    explicit UniquePtr(PrivateConstructorTag, Allocator* allocator, Args&&... args) :
        m_allocator(allocator) {
        m_buffer = static_cast<T*>(m_allocator->allocate(1));
        new (m_buffer) T(std::forward<Args>(args)...);
    }

    inline static Mallocator<T> s_defaultAllocator;

    Allocator* m_allocator;
    T* m_buffer;
};

}  // namespace sl
