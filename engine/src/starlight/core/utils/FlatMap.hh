#pragma once

#include <starlight/core/Core.hh>

#include "StableVector.hh"

namespace sl {

template <typename Key, typename T> class FlatMap {
    struct Record {
        explicit Record(const Key& key, T&& value) :
            key(key), value(std::move(value)) {}

        Key key;
        T value;
    };

public:
    static constexpr u64 defaultCapacity = 1024;

    explicit FlatMap(u64 capacity = defaultCapacity) : m_buffer(capacity) {}

    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    T* emplace(const Key& key, Args&&... args) {
        auto record = m_buffer.emplace(key, T{ std::forward<Args>(args)... });
        return &record->value;
    }

    bool has(const Key& key) {
        return m_buffer.has([&](auto& record) -> bool { return record.key == key; });
    }

    T* get(const Key& key) {
        auto record =
          m_buffer.find([&](auto& record) -> bool { return record.key == key; });
        return &record->value;
    }

    template <typename C>
    requires Callable<C, void, const Key&, T&>
    void forEach(C&& callback) {
        m_buffer.forEach([&](auto& record) { callback(record.key, record.value); });
    }

private:
    StableVector<Record> m_buffer;
};

}  // namespace sl
