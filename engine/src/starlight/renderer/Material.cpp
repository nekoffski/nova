#include "Material.hh"

#include "starlight/core/Json.hh"
#include "starlight/core/Enum.hh"
#include "starlight/core/Log.hh"

#include "starlight/renderer/factories/TextureFactory.hh"

namespace sl {

Material::Material(const Properties& props) :
    shininess(props.shininess), diffuseColor(props.diffuseColor),
    m_renderFrameNumber(0), m_textures(props.textures) {
    m_instance.emplace(m_textures.asArray());
    log::trace("Creating Material");
}

Material::~Material() { log::trace("Destroying Material: {}", getId()); }

bool Material::isTransparent() const {
    return isFlagEnabled(
      m_textures.diffuse->getImageData().flags, Texture::Flags::transparent
    );
}

void Material::applyUniforms(
  ResourceRef<Shader> shader, CommandBuffer& commandBuffer, u32 imageIndex,
  const u64 renderFrameNumber
) {
    if (m_renderFrameNumber != renderFrameNumber) {
        shader->setInstanceUniforms(
          commandBuffer, m_instance->getId(shader), imageIndex,
          [&](Shader::UniformProxy& proxy) {
              proxy.set("diffuseColor", diffuseColor);
              proxy.set("diffuseTexture", m_textures.diffuse);
              proxy.set("specularTexture", m_textures.specular);
              proxy.set("normalTexture", m_textures.normal);
              proxy.set("shininess", shininess);
          }
        );
        m_renderFrameNumber = renderFrameNumber;
    }
}

const Material::Textures& Material::getTextures() const { return m_textures; }

void Material::setTextures(const Material::Textures& textures) {
    m_textures = textures;
    m_instance.clear();
    m_instance.emplace(m_textures.asArray());
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

std::vector<Texture*> Material::Textures::asArray() {
    return std::vector<Texture*>{ diffuse.get(), specular.get(), normal.get() };
}

}  // namespace sl
