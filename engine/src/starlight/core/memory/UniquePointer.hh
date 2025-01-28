#pragma once

#include <concepts>
#include <utility>

#include "starlight/core/Core.hh"

#include "Allocator.hh"
#include "Mallocator.hh"

namespace sl {

template <typename T> class UniquePointer : public NonCopyable {
    template <typename C> friend class UniquePointer;

public:
    explicit UniquePointer() : m_allocator(nullptr), m_buffer(nullptr) {}
    UniquePointer(std::nullptr_t) : UniquePointer() {}

    ~UniquePointer() noexcept { clear(); }

    void clear() {
        if (m_buffer) {
            m_buffer->~T();
            m_allocator->deallocate(m_buffer, 1);
            m_buffer = nullptr;
        }
    }

    UniquePointer& operator=(UniquePointer&& oth) {
        clear();

        m_buffer    = std::exchange(oth.m_buffer, nullptr);
        m_allocator = std::exchange(oth.m_allocator, nullptr);

        return *this;
    }

    template <typename F>
    requires std::derived_from<F, T>
    UniquePointer(UniquePointer<F>&& oth) {
        m_buffer    = std::exchange(oth.m_buffer, nullptr);
        m_allocator = std::exchange(oth.m_allocator, nullptr);
    }

    UniquePointer(UniquePointer&& oth) {
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

    // template <typename C, typename... Args>
    // requires std::constructible_from<C, Args...>
    // friend UniquePointer<C> makeUniquePointer(Allocator* allocator, Args&&...
    // args);

    // template <typename C, typename... Args>
    // requires std::constructible_from<C, Args...>
    // friend UniquePointer<C> makeUniquePointer(Args&&... args);

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    static UniquePointer<T> create(Allocator* allocator, Args&&... args) {
        return UniquePointer<T>{ allocator, std::forward<Args>(args)... };
    }

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    static UniquePointer<T> create(Args&&... args) {
        return UniquePointer<T>{ &s_defaultAllocator, std::forward<Args>(args)... };
    }

private:
    template <typename... Args>
    requires std::constructible_from<T, Args...>
    explicit UniquePointer(Allocator* allocator, Args&&... args) :
        m_allocator(allocator) {
        m_buffer = static_cast<T*>(m_allocator->allocate(1));
        new (m_buffer) T(std::forward<Args>(args)...);
    }

    inline static Mallocator<T> s_defaultAllocator;

    Allocator* m_allocator;
    T* m_buffer;
};

}  // namespace sl
