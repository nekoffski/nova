#pragma once

#include <nlohmann/json.hpp>

#include "math/Core.hh"
#include "Concepts.hh"

namespace glm {

template <typename T>
concept GlmRandomAccessIterable = requires(T value) {
    { value[0] };
    { glm::value_ptr(value) };
};

template <typename T>
requires GlmRandomAccessIterable<T>
void from_json(const nlohmann::json& j, T& v) {
    auto iterator = glm::value_ptr(v);
    for (sl::u64 i = 0; i < sl::getSize<T>(); ++i) iterator[i] = j[i].get<f32>();
}

template <typename T>
requires GlmRandomAccessIterable<T>
void to_json(nlohmann::json& j, const T& v) {
    auto iterator = glm::value_ptr(v);
    for (sl::u64 i = 0; i < sl::getSize<T>(); ++i) j.push_back(iterator[i]);
}

}  // namespace glm

namespace sl {

namespace json {

bool hasField(const nlohmann::json& j, std::string_view field);

template <typename T>
void getIfExists(const nlohmann::json& j, std::string_view field, T& out) {
    if (hasField(j, field)) j.at(field).get_to(out);
}

template <typename T>
T getOr(const nlohmann::json& j, std::string_view field, const T& fallback) {
    return hasField(j, field) ? j.at(field).get<T>() : fallback;
}

}  // namespace json

template <typename T>
concept Serializable = requires(nlohmann::json& j, const T& v) {
    { serialize(j, v) };
};

template <typename T>
requires Serializable<T>
void to_json(nlohmann::json& j, const T& v) {
    serialize(j, v);
}

template <typename T>
concept Deserializable = requires(const nlohmann::json& j, T& v) {
    { deserialize(j, v) };
};

template <typename T>
requires Deserializable<T>
void from_json(const nlohmann::json& j, T& v) {
    deserialize(j, v);
}

}  // namespace sl
