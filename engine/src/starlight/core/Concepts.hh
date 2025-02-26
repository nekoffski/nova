#pragma once

#include <string>
#include <concepts>
#include <utility>

namespace sl {

template <typename T>
concept HasToString = requires(const T& value) {
    { toString(value) } -> std::convertible_to<std::string>;
};

template <typename T>
concept HasName = requires(const T& value) {
    { value.name } -> std::convertible_to<std::string>;
};

template <typename C, typename R = void, typename... Args>
concept Callable = requires(C&& callback, Args&&... args) {
    { callback(std::forward<Args>(args)...) } -> std::same_as<R>;
};

template <typename T, typename... Ts>
concept OneOf = requires() {
    { (std::is_same_v<T, Ts> || ...) };
};

}  // namespace sl
