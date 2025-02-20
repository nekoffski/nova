#include "DirectionalLight.hh"

#include <fmt/format.h>

namespace sl {

std::string toString(const DirectionalLight& l) {
    return fmt::format(
      "DirectionalLight[{}b]: color={}, direction={}", sizeof(DirectionalLight),
      l.color, l.direction
    );
}

}  // namespace sl