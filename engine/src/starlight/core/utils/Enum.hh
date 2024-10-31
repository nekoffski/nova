#pragma once

#include <concepts>

namespace sl {

template <typename T>
requires(std::is_enum_v<T> && requires(T e) { enableBitOperations(e); })
constexpr auto operator|(const T lhs, const T rhs) {
    using underlying = std::underlying_type_t<T>;

    return static_cast<T>(
      static_cast<underlying>(lhs) | static_cast<underlying>(rhs)
    );
}

template <typename T>
requires(std::is_enum_v<T> && requires(T e) { enableBitOperations(e); })
constexpr auto operator&(const T lhs, const T rhs) {
    using underlying = std::underlying_type_t<T>;

    return static_cast<T>(
      static_cast<underlying>(lhs) & static_cast<underlying>(rhs)
    );
}

template <typename T>
requires(std::is_enum_v<T> && requires(T e) { enableBitOperations(e); })
constexpr bool isFlagEnabled(const T lhs, const T rhs) {
    using underlying = std::underlying_type_t<T>;
    return static_cast<underlying>(lhs & rhs) > 0;
}

template <typename T>
requires(std::is_enum_v<T> && requires(T e) { enableBitOperations(e); })
constexpr auto operator|=(T& a, T b) {
    return a = a | b;
}

}  // namespace sl
