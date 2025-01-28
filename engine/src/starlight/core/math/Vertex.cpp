#include "Vertex.hh"

#include "starlight/core/Utils.hh"

#include <iostream>

namespace sl {

std::string toString(const Vertex3& v) {
    return fmt::format(
      "Vertex3: position={}, texture coordinates={}, normal={}, color={}, tangent = {}",
      v.position, v.textureCoordinates, v.normal, v.color, v.tangent
    );
}

}  // namespace sl
