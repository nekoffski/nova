#pragma once

#include <vector>
#include <algorithm>

#include <starlight/core/Core.hh>

namespace sl {

template <typename T, typename V, typename C>
requires Callable<C, T, const V&>
std::vector<T> transform(const std::vector<V>& in, C&& callback) {
    std::vector<T> out;
    out.reserve(in.size());

    std::transform(
      in.begin(), in.end(), std::back_inserter(out), std::move(callback)
    );

    return out;
}

}  // namespace sl
