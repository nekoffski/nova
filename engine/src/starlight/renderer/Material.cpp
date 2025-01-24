#include "Material.hh"

#include "starlight/core/utils/Json.hh"
#include "starlight/core/utils/Enum.hh"

#include "starlight/renderer/factories/TextureFactory.hh"

namespace sl {

Material::Material(const Properties& props) :
    shininess(props.shininess), diffuseColor(props.diffuseColor),
    m_renderFrameNumber(0),
    m_textures{ props.diffuseMap, props.specularMap, props.normalMap } {
    m_instance.emplace(m_textures.asArray());
    LOG_TRACE("Creating Material");
}

Material::~Material() { LOG_TRACE("Destroying Material: {}", getId()); }

bool Material::isTransparent() const {
    return isFlagEnabled(
      m_textures.diffuseMap->getImageData().flags, Texture::Flags::transparent
    );
}

void Material::applyUniforms(
  ResourceRef<Shader> shader, CommandBuffer& commandBuffer, u32 imageIndex,
  const u64 renderFrameNumber
) {
    if (true) {
        //(m_renderFrameNumber != renderFrameNumber) {
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
    LOG_TRACE("Loading material properties file: {}", path);

    if (not fs.isFile(path)) {
        LOG_ERROR("Could not find file: '{}'", path);
        return {};
    }

    try {
        const auto root = kc::json::loadJson(fs.readFile(path));

        auto props = Properties::createDefault();

        auto& textureManager = TextureFactory::get();

        getOptField<Vec4<f32>>(root, "diffuse-color", [&](const auto& value) {
            props.diffuseColor = value;
        });
        getOptField<std::string>(root, "diffuse-map", [&](const auto& value) {
            props.diffuseMap = textureManager.load(value, Texture::Type::flat);
        });
        getOptField<std::string>(root, "specular-map", [&](const auto& value) {
            props.specularMap = textureManager.load(value, Texture::Type::flat);
        });
        getOptField<std::string>(root, "normal-map", [&](const auto& value) {
            props.normalMap = textureManager.load(value, Texture::Type::flat);
        });
        getOptField<float>(root, "shininess", [&](const auto& value) {
            props.shininess = value;
        });

        return props;
    } catch (kc::json::JsonError& e) {
        LOG_ERROR("Could not parse material '{}' file: {}", path, e.asString());
    }
    return {};
}

std::vector<Texture*> Material::Textures::asArray() {
    return std::vector<Texture*>{
        diffuseMap.get(), specularMap.get(), normalMap.get()
    };
}

}  // namespace sl
