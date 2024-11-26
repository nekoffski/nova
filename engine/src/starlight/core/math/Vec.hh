#pragma once

#include <kc/math/Format.h>

#include "starlight/core/Core.hh"
#include "Glm.hh"

namespace sl {

namespace detail {

template <typename T> struct Vec2Base {
    explicit Vec2Base(T x = 0, T y = 0) : x(x), y(y), w(this->x), h(this->y) {}
    Vec2Base(const Vec2Base& oth) : Vec2Base(oth.x, oth.y) {}

    Vec2Base& operator=(const Vec2Base& oth) {
        x = oth.x;
        y = oth.y;
        return *this;
    }

    T x;
    T y;

    T& w;
    T& h;
};

template <typename T> struct Vec3Base {
    explicit Vec3Base(T x = 0, T y = 0, T z = 0) :
        x(x), y(y), z(z), w(this->x), h(this->y), d(this->z) {}
    Vec3Base(const Vec3Base& oth) : Vec3Base(oth.x, oth.y, oth.z) {}

    Vec3Base& operator=(const Vec3Base& oth) {
        x = oth.x;
        y = oth.y;
        z = oth.z;
        return *this;
    }

    T x;
    T y;
    T z;

    T& w;
    T& h;
    T& d;
};

template <typename T> struct Vec4Base {
    explicit Vec4Base(T x = 0, T y = 0, T z = 0, T w = 0) : x(x), y(y), z(z), w(w) {}
    Vec4Base(const Vec4Base& oth) : Vec4Base(oth.x, oth.y, oth.z, oth.w) {}

    Vec4Base& operator=(const Vec4Base& oth) {
        x = oth.x;
        y = oth.y;
        z = oth.z;
        w = oth.w;
        return *this;
    }

    T x;
    T y;
    T z;
    T w;
};

template <typename T> struct Mat2Picker {};
template <> struct Mat2Picker<f32> {
    using Type = math::mat2;
};

template <typename T> struct Mat3Picker {};
template <> struct Mat3Picker<f32> {
    using Type = math::mat3;
};

template <typename T> struct Mat4Picker {};
template <> struct Mat4Picker<f32> {
    using Type = math::mat4;
};

template <typename T> struct Vec2Picker {
    using Type = Vec2Base<T>;
};
template <> struct Vec2Picker<f32> {
    using Type = math::vec2;
};

template <typename T> struct Vec3Picker {
    using Type = Vec3Base<T>;
};
template <> struct Vec3Picker<f32> {
    using Type = math::vec3;
};

template <typename T> struct Vec4Picker {
    using Type = Vec4Base<T>;
};
template <> struct Vec4Picker<f32> {
    using Type = math::vec4;
};

}  // namespace detail

template <typename T> using Vec2 = detail::Vec2Picker<T>::Type;
template <typename T> using Vec3 = detail::Vec3Picker<T>::Type;
template <typename T> using Vec4 = detail::Vec4Picker<T>::Type;

template <typename T> using Mat2 = detail::Mat2Picker<T>::Type;
template <typename T> using Mat3 = detail::Mat3Picker<T>::Type;
template <typename T> using Mat4 = detail::Mat4Picker<T>::Type;

inline Vec3<f32> operator*(const Mat4<f32>& lhs, const Vec3<f32>& rhs) {
    return Vec3<f32>{
        lhs * Vec4<f32>{ rhs, 1.0f }
    };
}

const Vec3<f32> worldUp    = Vec3<f32>{ 0.0f, 1.0f, 0.0f };
const Vec3<f32> worldRight = Vec3<f32>{ 1.0f, 0.0f, 0.0 };
const Vec3<f32> worldFront = Vec3<f32>{ 0.0f, 0.0f, 1.0f };

}  // namespace sl
