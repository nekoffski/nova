#include "Log.hh"

#include <fmt/core.h>

namespace sl::log {

void init(std::string_view applicationName) {
    const auto pattern =
      fmt::format("%^[%d-%m-%Y %T] [Th: %t] %-7l [{}]: %v %$", applicationName);
    spdlog::set_pattern(pattern);
    spdlog::set_level(spdlog::level::trace);
}

}  // namespace sl::log
