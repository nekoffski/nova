#pragma once

#include <cstdint>
#include <cstring>

#include <limits>
#include <string>
#include <concepts>

#include <kc/core/ErrorBase.hpp>
#include <kc/core/Scope.hpp>

namespace sl {

DEFINE_ERROR(Error);

struct NonCopyable {
    NonCopyable()                         = default;
    NonCopyable(NonCopyable&&)            = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};

struct NonMovable {
    NonMovable()                             = default;
    NonMovable(NonMovable const&)            = delete;
    NonMovable& operator=(NonMovable const&) = delete;
};

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8  = uint8_t;

using Nanoseconds = u64;

constexpr auto u64Max = std::numeric_limits<u64>::max();

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8  = int8_t;

using f32 = float;
using f64 = double;

struct Range {
    u64 offset;
    u64 size;

    static Range aligned(u64 offset, u64 size, u64 granularity);
};

template <typename T> struct Interval {
    T min;
    T max;
};

template <typename T> struct StepInterval {
    T min;
    T max;
    T step;
};

using Intervalf   = Interval<float>;
using Intervalu32 = Interval<u32>;
using Intervalu64 = Interval<u64>;

std::string_view removeExtension(std::string_view path);

template <typename T> T getAlignedValue(T value, T granularity) {
    return (value + granularity - 1) & ~(granularity - 1);
}

template <typename C, typename R = void, typename... Args>
concept Callable = requires(C&& callback, Args&&... args) {
    { callback(std::forward<Args>(args)...) } -> std::same_as<R>;
};

template <typename T> inline void clearMemory(T* target) {
    std::memset(target, 0, sizeof(T));
}

}  // namespace sl
