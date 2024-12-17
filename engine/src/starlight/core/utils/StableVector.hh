#pragma once

#include <vector>
#include <queue>

#include "starlight/core/Core.hh"
#include "starlight/core/Log.hh"
#include "starlight/core/memory/Memory.hh"

#include <iostream>

namespace sl {

template <typename T> class StableVector {
public:
    explicit StableVector(u64 capacity
    ) : m_capacity(capacity), m_buffer(m_capacity) {
        for (u64 i = 0; i < capacity; ++i) m_freeSlots.push(i);
    }

    void clear() {
        for (auto& slot : m_buffer) slot.clear();
    }

    template <typename C>
    requires Callable<C, void, T&>
    void forEach(C&& callback) {
        for (auto& slot : m_buffer)
            if (slot) callback(*slot);
    }

    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    T* emplace(Args&&... args) {
        if (m_freeSlots.empty()) return nullptr;

        auto& slot = m_buffer[m_freeSlots.front()];
        m_freeSlots.pop();

        slot.emplace(std::forward<Args>(args)...);
        return slot.get();
    }

    template <typename C>
    requires Callable<C, bool, const T&>
    T* find(C&& condition) {
        for (auto& record : m_buffer)
            if (record && condition(*record)) return record.get();
        return nullptr;
    }

    template <typename C>
    requires Callable<C, bool, const T&>
    bool has(C&& condition) {
        for (auto& record : m_buffer)
            if (record && condition(*record)) return true;
        return false;
    }

    bool erase(T* value) {
        for (u64 i = 0; i < m_capacity; ++i) {
            if (auto& slot = m_buffer[i]; slot && slot.get() == value) {
                m_freeSlots.push(i);
                slot.clear();
                return true;
            }
        }
        return false;
    }

    u64 getCapacity() const { return m_capacity; }
    u64 getSize() const { return m_capacity - m_freeSlots.size(); }

private:
    const u64 m_capacity;
    std::vector<LocalPtr<T>> m_buffer;
    std::queue<u64> m_freeSlots;
};

}  // namespace sl
