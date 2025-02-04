#pragma once

#include <vector>
#include <algorithm>

#include <fmt/core.h>
#include <fmt/ostream.h>

#include "Concepts.hh"
#include "Core.hh"

namespace sl {

template <typename T> T fromString(std::string_view str) { return T{ str }; }

template <typename T>
requires HasToString<T>
std::ostream& operator<<(std::ostream& stream, const T& value) {
    stream << toString(value);
    return stream;
}

template <class... Ts> struct Overload : Ts... {
    using Ts::operator()...;
};

template <typename T> struct Interval {
    T min;
    T max;
};

template <typename T> struct StepInterval {
    T min;
    T max;
    T step;
};

using Intervalf   = Interval<float>;
using Intervalu32 = Interval<u32>;
using Intervalu64 = Interval<u64>;

template <typename T> T getAlignedValue(T value, T granularity) {
    return (value + granularity - 1) & ~(granularity - 1);
}

template <typename T> inline void clearMemory(T* target) {
    std::memset(target, 0, sizeof(T));
}

struct Range {
    u64 offset;
    u64 size;

    static Range aligned(u64 offset, u64 size, u64 granularity);
};

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

template <
  typename T, typename Allocator, template <typename, typename> typename Container>
bool contains(const Container<T, Allocator>& container, const T& value) {
    return std::find(container.begin(), container.end(), value) != container.end();
}

std::vector<std::string> split(const std::string& input, char separator);

template <typename Allocator, template <typename, typename> typename Container>
bool contains(
  const Container<const char*, Allocator>& container, const char* const& value
) {
    auto comparator = [value](const char* oth) {
        return std::strcmp(oth, value) == 0;
    };
    return std::find_if(container.begin(), container.end(), comparator)
           != container.end();
}

}  // namespace sl

namespace fmt {

template <typename T>
requires sl::HasToString<T>
struct formatter<T> : ostream_formatter {};

}  // namespace fmt
