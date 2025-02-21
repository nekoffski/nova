#pragma once

#include <atomic>
#include <concepts>

#include "Allocator.hh"
#include "starlight/core/Core.hh"

/*
    TODOs:
        - support for allocators
        - allocate both control block and resource in a single allocation
*/

namespace sl {
namespace detail {

struct ControlBlock {
    std::atomic<i64> referenceCounter = 1;
};

}  // namespace detail

template <typename T> class SharedPointer : public NonCopyable {
    template <typename C> friend class SharedPointer;

    struct PrivateConstructorTag {};

public:
    class View {
    public:
        View(SharedPointer& ptr) : m_ptr(ptr) {}

        SharedPointer clone() { return m_ptr.clone(); }
        operator SharedPointer() { return m_ptr.clone(); }

    private:
        SharedPointer& m_ptr;
    };

    explicit SharedPointer() : m_controlBlock(nullptr), m_buffer(nullptr) {}
    SharedPointer(std::nullptr_t) : SharedPointer() {}

    ~SharedPointer() { reset(); }

    template <typename F>
    requires std::derived_from<F, T>
    SharedPointer(SharedPointer<F>&& oth) {
        m_buffer       = std::exchange(oth.m_buffer, nullptr);
        m_controlBlock = std::exchange(oth.m_controlBlock, nullptr);
    }

    void reset() {
        if (m_controlBlock && m_controlBlock->referenceCounter.fetch_sub(1) == 1) {
            delete m_controlBlock;
            delete m_buffer;
        }
    }

    bool empty() const { return m_buffer == nullptr; }
    operator bool() const { return not empty(); }

    T& operator*() { return *m_buffer; }
    const T& operator*() const { return *m_buffer; }

    T* operator->() { return m_buffer; }
    const T* operator->() const { return m_buffer; }

    T* get() { return m_buffer; }
    const T* get() const { return m_buffer; }

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    static SharedPointer create(Args&&... args) {
        return SharedPointer<T>{
            PrivateConstructorTag{}, std::forward<Args>(args)...
        };
    }

    SharedPointer clone() { return SharedPointer{ m_controlBlock, m_buffer }; }

private:
    explicit SharedPointer(detail::ControlBlock* controlBlock, T* buffer) :
        m_controlBlock(controlBlock), m_buffer(buffer) {
        m_controlBlock->referenceCounter++;
    }

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    explicit SharedPointer(PrivateConstructorTag, Args&&... args) :
        m_controlBlock(new detail::ControlBlock),
        m_buffer(new T(std::forward<Args>(args)...)) {}

    detail::ControlBlock* m_controlBlock;
    T* m_buffer;
};

}  // namespace sl
