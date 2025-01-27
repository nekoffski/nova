#pragma once

#include <mutex>
#include <limits>
#include <concepts>
#include <queue>

#include "starlight/core/Core.hh"

namespace sl {

template <typename T, typename Id = u64>
requires std::is_arithmetic_v<Id>
class Identificable : public virtual NonCopyable {
public:
    explicit Identificable() : m_id(createId()) {}

    Identificable(Identificable&& oth)            = default;
    Identificable& operator=(Identificable&& oth) = default;

    ~Identificable() {
        std::scoped_lock guard{ s_mutex };
        s_freeIds.push(m_id);
    }

    Id getId() const { return m_id; }

private:
    static Id createId() {
        std::scoped_lock guard{ s_mutex };

        if (not s_freeIds.empty()) {
            const auto id = s_freeIds.front();
            s_freeIds.pop();
            return id;
        }

        return s_generator++;
    }

    Id m_id;

    inline static Id s_generator = 0;
    inline static std::queue<Id> s_freeIds;
    inline static std::mutex s_mutex;
};

template <typename T>
concept IsIdentificable = std::derived_from<T, Identificable<T>>;

template <typename T>
requires std::is_integral_v<T>
class Id {
    static constexpr T invalidId = std::numeric_limits<T>::max();

public:
    using Type = T;

    Id() : m_value(invalidId) {}
    Id(T value) : m_value(value) {}

    T operator*() const { return get(); }
    T get() const { return m_value; }

    Id& operator=(T value) {
        m_value = value;
        return *this;
    }

    void invalidate() { m_value = invalidId; }

    bool hasValue() const { return m_value != invalidId; }

private:
    T m_value;
};

using Id8  = Id<u8>;
using Id16 = Id<u16>;
using Id32 = Id<u32>;
using Id64 = Id<u64>;

}  // namespace sl
