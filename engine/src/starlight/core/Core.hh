#pragma once

#include <cstdint>
#include <cstring>

#include <limits>
#include <string>

namespace sl {

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

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8  = int8_t;

using f32 = float;
using f64 = double;

template <typename T> T max() { return std::numeric_limits<T>::max(); }

}  // namespace sl
