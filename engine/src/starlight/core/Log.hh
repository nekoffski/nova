#pragma once

#include <stdlib.h>

#include <string_view>

#include <spdlog/spdlog.h>

namespace sl::log {

namespace detail {
template <typename... Args>
[[noreturn]] constexpr inline void abort(
  spdlog::format_string_t<Args...> fmt, Args&&... args
) {
    spdlog::error(fmt, std::forward<Args>(args)...);
    std::abort();
}
}  // namespace detail

void init(std::string_view applicationName);

using spdlog::debug;
using spdlog::error;
using spdlog::info;
using spdlog::trace;
using spdlog::warn;

template <typename... Args>
[[noreturn]] constexpr inline void panic(
  spdlog::format_string_t<Args...> fmt, Args&&... args
) {
    error("PANIC!");
    detail::abort(std::move(fmt), std::forward<Args>(args)...);
}

template <typename... Args>
constexpr inline void expect(
  bool condition, spdlog::format_string_t<Args...> fmt, Args&&... args
) {
    if (not condition) [[unlikely]] {
        error("ASSERTION FAILED");
        detail::abort(std::move(fmt), std::forward<Args>(args)...);
    }
}

}  // namespace sl::log
