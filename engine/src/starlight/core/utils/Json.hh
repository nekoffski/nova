#pragma once

#include <concepts>

#include <kc/json/Json.h>
#include <kc/json/Utils.hpp>

#include "starlight/core/math/Core.hh"

namespace sl {

template <typename T>
concept GlmRandomAccessIterable = requires(T value) {
    { value.length() } -> std::convertible_to<u64>;
    { value[0] };
};

void addJsonField(
  kc::json::Node& node, const std::string& name,
  const GlmRandomAccessIterable auto& value
) {
    kc::json::Node element;

    static constexpr auto getLength = [](const auto& value) -> u64 {
        if constexpr (requires { value[0][0]; })
            return value.length() * value.length();
        else
            return value.length();
    };

    if constexpr (requires { math::value_ptr(value); }) {
        const auto iterator = math::value_ptr(value);
        for (int i = 0; i < getLength(value); ++i) element[i] = iterator[i];
    } else {
        for (int i = 0; i < getLength(value); ++i) element[i] = value[i];
    }

    node[name] = element;
}

auto fieldFrom(auto& root) { return kc::json::fieldFrom<kc::json::JsonError>(root); }

template <typename T> T getField(auto& root, const std::string& name) {
    return fieldFrom(root).withName(name).template ofType<T>().get();
}

template <typename T>
T getFieldOr(auto& root, const std::string& name, const T& defaultValue) {
    if (root.isMember(name)) return getField<T>(root, name);
    return defaultValue;
}

template <typename T, typename C>
requires Callable<C, void, const T&>
void getOptField(auto& root, const std::string& name, C&& callback) {
    if (root.isMember(name)) callback(getField<T>(root, name));
}

auto getArray(auto& root, const std::string& name) {
    return fieldFrom(root).withName(name).asArray().get();
}

}  // namespace sl
