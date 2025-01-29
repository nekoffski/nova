#pragma once

#include <tuple>
#include <optional>
#include <concepts>
#include <cmath>

namespace sl {

template <typename T>
requires std::is_floating_point_v<T>
std::optional<std::pair<T, T>> solveQuadraticEquation(T a, T b, T c) {
    T delta = b * b - 4 * a * c;
    if (delta < 0) {
        return {};
    } else if (delta == 0) {
        T x = -0.5 * b / a;
        return std::make_pair(x, x);
    }

    T deltaSqrt = std::sqrt(delta);
    T q         = (b > 0) ? -0.5 * (b + deltaSqrt) : -0.5 * (b - deltaSqrt);

    T x0 = q / a;
    T x1 = c / q;

    if (x0 > x1) std::swap(x0, x1);

    return std::make_pair(x0, x1);
}

}  // namespace sl
