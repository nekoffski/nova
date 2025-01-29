#include "Material.hh"

#include "starlight/core/Json.hh"
#include "starlight/core/Enum.hh"
#include "starlight/core/Log.hh"

#include "starlight/renderer/factories/TextureFactory.hh"

namespace sl {

Material::Material(const Properties& props) :
    shininess(props.shininess), diffuseColor(props.diffuseColor),
    m_renderFrameNumber(0),
    m_textures{ props.diffuseMap, props.specularMap, props.normalMap } {
    m_instance.emplace(m_textures.asArray());
    log::trace("Creating Material");
}

Material::~Material() { log::trace("Destroying Material: {}", getId()); }

bool Material::isTransparent() const {
    return isFlagEnabled(
      m_textures.diffuseMap->getImageData().flags, Texture::Flags::transparent
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
              proxy.set("diffuseTexture", m_textures.diffuseMap);
              proxy.set("specularTexture", m_textures.specularMap);
              proxy.set("normalTexture", m_textures.normalMap);
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

    return Properties{
        .diffuseColor = defaultDiffuseColor,
        .diffuseMap   = textureManager.getDefaultDiffuseMap(),
        .specularMap  = textureManager.getDefaultSpecularMap(),
        .normalMap    = textureManager.getDefaultNormalMap(),
        .shininess    = defaultShininess
    };
}

std::optional<Material::Properties> Material::Properties::fromFile(
  const std::string& path, const FileSystem& fs
) {
    log::trace("Loading material properties file: {}", path);

    if (not fs.isFile(path)) {
        log::error("Could not find file: '{}'", path);
        return {};
    }

    try {
        const auto root = nlohmann::json::parse(fs.readFile(path));

        auto props = Properties::createDefault();

        json::getIfExists(root, "diffuse-color", props.diffuseColor);
        json::getIfExists(root, "diffuse-map", props.diffuseMap);
        json::getIfExists(root, "specular-map", props.specularMap);
        json::getIfExists(root, "normal-map", props.normalMap);
        json::getIfExists(root, "shininess", props.shininess);

        return props;
    } catch (const nlohmann::json::parse_error& e) {
        log::error("Could not parse material '{}' file: {}", path, e.what());
    }
    return {};
}

std::vector<Texture*> Material::Textures::asArray() {
    return std::vector<Texture*>{
        diffuseMap.get(), specularMap.get(), normalMap.get()
    };
}

}  // namespace sl
