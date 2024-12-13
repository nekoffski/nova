#include "DirectionalLight.hh"

#include <fmt/format.h>

namespace sl {

std::string DirectionalLight::toString() const {
    return fmt::format(
      "DirectionalLight[{}b]: color={}, direction={}", sizeof(DirectionalLight),
      color, direction
    );
}

}  // namespace sl