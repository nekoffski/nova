#include "Vec.hh"

namespace sl {

std::string toString(const Vec2<f32>& vector) {
    return fmt::format("[{}, {}]", vector.x, vector.y);
}

std::string toString(const Vec3<f32>& vector) {
    return fmt::format("[{}, {}, {}]", vector.x, vector.y, vector.z);
}

std::string toString(const Vec4<f32>& vector) {
    return fmt::format("[{}, {}, {}, {}]", vector.x, vector.y, vector.z, vector.w);
}

}  // namespace sl