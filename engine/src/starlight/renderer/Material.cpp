#include "Material.hh"

#include "starlight/core/Log.hh"

namespace sl {

Material::Material(const Properties& props) :
    diffuseMap(props.diffuseMap), specularMap(props.specularMap),
    normalMap(props.normalMap), shininess(props.shininess),
    diffuseColor(props.diffuseColor) {
    log::trace("Creating Material: {}", getId());
}

Material::~Material() { log::trace("Destroying Material: {}", getId()); }

bool Material::isTransparent() const {
    return isFlagEnabled(
      diffuseMap->getImageData().flags, Texture::Flags::transparent
    );
}

}  // namespace sl
