#include "Glm.hh"

#include <fmt/core.h>

namespace sl::math {

std::string toString(const glm::vec2& vector) {
    return fmt::format("[{}, {}]", vector.x, vector.y);
}

std::string toString(const glm::vec3& vector) {
    return fmt::format("[{}, {}, {}]", vector.x, vector.y, vector.z);
}

std::string toString(const glm::vec4& vector) {
    return fmt::format("[{}, {}, {}, {}]", vector.x, vector.y, vector.z, vector.w);
}

}  // namespace sl::math