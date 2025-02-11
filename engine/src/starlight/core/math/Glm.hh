#pragma once

#include <string>

#include "starlight/core/Utils.hh"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace sl::math {

using namespace glm;

std::string toString(const glm::vec2& vector);
std::string toString(const glm::vec3& vector);
std::string toString(const glm::vec4& vector);

}  // namespace sl::math

namespace glm {

inline std::string toString(const vec4& v) { return sl::math::toString(v); }
inline std::string toString(const vec3& v) { return sl::math::toString(v); }
inline std::string toString(const vec2& v) { return sl::math::toString(v); }

inline std::string toString(const mat4& v) {
    return fmt::format("[ {} ]\n[ {} ]\n[ {} ]\n[ {} ]", v[0], v[1], v[2], v[3]);
}

template <typename T>
requires sl::HasToString<T>
std::ostream& operator<<(std::ostream& stream, const T& value) {
    stream << toString(value);
    return stream;
}

}  // namespace glm
