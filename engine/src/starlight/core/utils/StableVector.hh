#pragma once

#include <vector>

#include "starlight/core/Core.hh"
#include "starlight/core/Log.hh"
#include "starlight/core/memory/Memory.hh"

#include <iostream>

namespace sl {

template <typename T> class StableVector {
public:
    explicit StableVector(u64 capacity
    ) : m_capacity(capacity), m_size(0u), m_buffer(m_capacity) {}

    template <typename C>
    requires Callable<C, void, T&>
    void forEach(C&& callback) {
        for (auto& slot : m_buffer)
            if (slot) callback(*slot);
    }

    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    T* emplace(Args&&... args) {
        for (auto& slot : m_buffer) {
            if (not slot) {
                ++m_size;
                slot.emplace(std::forward<Args>(args)...);
                return slot.get();
            }
        }
        return nullptr;
    }

    bool erase(T* value) {
        for (auto& slot : m_buffer) {
            if (slot && slot.get() == value) {
                --m_size;
                slot.clear();
                return true;
            }
        }
        return false;
    }

    u64 getCapacity() const { return m_capacity; }
    u64 getSize() const { return m_size; }

private:
    const u64 m_capacity;
    u64 m_size;
    std::vector<LocalPtr<T>> m_buffer;
};

}  // namespace sl
