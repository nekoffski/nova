#include "Material.hh"

#include "starlight/core/Log.hh"

namespace sl {

Material::Material(const Properties& props, OptStr name) :
    NamedResource(name), diffuseMap(props.diffuseMap),
    specularMap(props.specularMap), normalMap(props.normalMap),
    shininess(props.shininess), diffuseColor(props.diffuseColor) {}

bool Material::isTransparent() const {
    return isFlagEnabled(
      diffuseMap->getImageData().flags, Texture::Flags::transparent
    );
}

}  // namespace sl
