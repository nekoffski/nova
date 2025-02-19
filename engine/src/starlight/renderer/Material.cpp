#include "Material.hh"

#include "starlight/core/Json.hh"
#include "starlight/core/Enum.hh"
#include "starlight/core/Log.hh"

#include "starlight/renderer/factories/TextureFactory.hh"

namespace sl {

Material::Material(const Properties& props) :
    shininess(props.shininess), diffuseColor(props.diffuseColor),
    textures(props.textures) {
    log::trace("Creating Material: {}", getId());
}

Material::~Material() { log::trace("Destroying Material: {}", getId()); }

bool Material::isTransparent() const {
    return isFlagEnabled(
      textures.diffuse->getImageData().flags, Texture::Flags::transparent
    );
}

Material::Properties Material::Properties::createDefault() {
    auto& textureManager = TextureFactory::get();

    // clang-format off
    return Properties{
        .diffuseColor = defaultDiffuseColor,
        .textures {
            .diffuse  = textureManager.getDefaultDiffuseMap(),
            .specular = textureManager.getDefaultSpecularMap(),
            .normal   = textureManager.getDefaultNormalMap(),
        },
        .shininess = defaultShininess
    };
    // clang-format on
}

}  // namespace sl
