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

template <typename T> class SharedPtr {
    template <typename C> friend class SharedPtr;

    struct PrivateConstructorTag {};

public:
    explicit SharedPtr() : m_controlBlock(nullptr), m_buffer(nullptr) {}
    SharedPtr(std::nullptr_t) : SharedPtr() {}

    ~SharedPtr() { reset(); }

    template <typename F>
    requires std::derived_from<F, T>
    SharedPtr& operator=(SharedPtr<F>&& oth) {
        reset();

        m_buffer       = std::exchange(oth.m_buffer, nullptr);
        m_controlBlock = std::exchange(oth.m_controlBlock, nullptr);

        return *this;
    }

    SharedPtr& operator=(const SharedPtr& oth) {
        reset();

        m_buffer       = oth.m_buffer;
        m_controlBlock = oth.m_controlBlock;

        if (m_controlBlock) m_controlBlock->referenceCounter++;

        return *this;
    }

    template <typename F>
    requires std::derived_from<F, T>
    SharedPtr(const SharedPtr<F>& oth
    ) : m_controlBlock(oth.m_controlBlock), m_buffer(oth.m_buffer) {
        if (m_controlBlock) m_controlBlock->referenceCounter++;
    }

    SharedPtr(const SharedPtr<T>& oth
    ) : m_controlBlock(oth.m_controlBlock), m_buffer(oth.m_buffer) {
        if (m_controlBlock) m_controlBlock->referenceCounter++;
    }

    template <typename F>
    requires std::derived_from<F, T>
    SharedPtr(SharedPtr<F>&& oth) {
        m_buffer       = std::exchange(oth.m_buffer, nullptr);
        m_controlBlock = std::exchange(oth.m_controlBlock, nullptr);
    }

    void reset() {
        if (m_controlBlock && m_controlBlock->referenceCounter.fetch_sub(1) == 1) {
            delete m_controlBlock;
            delete m_buffer;
        }
        m_controlBlock = nullptr;
        m_buffer       = nullptr;
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
    static SharedPtr create(Args&&... args) {
        return SharedPtr<T>{ PrivateConstructorTag{}, std::forward<Args>(args)... };
    }

private:
    template <typename... Args>
    requires std::constructible_from<T, Args...>
    explicit SharedPtr(PrivateConstructorTag, Args&&... args) :
        m_controlBlock(new detail::ControlBlock),
        m_buffer(new T(std::forward<Args>(args)...)) {}

    detail::ControlBlock* m_controlBlock;
    T* m_buffer;
};

}  // namespace sl
