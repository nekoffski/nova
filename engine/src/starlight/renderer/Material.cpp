#include "Material.hh"

#include "starlight/core/utils/Json.hh"

namespace sl {

Material::Material(const Properties& props
) : m_props(props), m_renderFrameNumber(0) {
    m_instance.emplace(std::vector<Texture*>{
      m_props.diffuseMap.get(),
      m_props.specularMap.get(),
      m_props.normalMap.get(),
    });
    LOG_TRACE("Creating Material");
}

Material::~Material() { LOG_TRACE("Destroying Material"); }

bool Material::isTransparent() const {
    return m_props.diffuseMap->getProperties().isTransparent;
}

void Material::applyUniforms(
  ResourceRef<Shader> shader, CommandBuffer& commandBuffer, u32 imageIndex,
  const u64 renderFrameNumber
) {
    if (m_renderFrameNumber != renderFrameNumber) {
        shader->setInstanceUniforms(
          commandBuffer, m_instance->getId(shader), imageIndex,
          [&](Shader::UniformProxy& proxy) {
              proxy.set("diffuseColor", m_props.diffuseColor);
              proxy.set("diffuseTexture", m_props.diffuseMap);
              proxy.set("specularTexture", m_props.specularMap);
              proxy.set("normalTexture", m_props.normalMap);
              proxy.set("shininess", m_props.shininess);
          }
        );
        m_renderFrameNumber = renderFrameNumber;
    }
}

const Material::Properties& Material::getProperties() const { return m_props; }

MaterialManager::MaterialManager(const std::string& path) :
    ResourceManager("Material"), m_materialsPath(path),
    m_defaultMaterial(create("DefaultMaterial")) {}

ResourceRef<Material> MaterialManager::getDefault() { return m_defaultMaterial; }

ResourceRef<Material> MaterialManager::create(
  const std::string& name, const Material::Properties& properties
) {
    return store(name, createOwningPtr<Material>(properties));
}

ResourceRef<Material> MaterialManager::create(const Material::Properties& properties
) {
    return store(createOwningPtr<Material>(properties));
}

ResourceRef<Material> MaterialManager::load(
  const std::string& name, const FileSystem& fs
) {
    const auto fullPath = fmt::format("{}/{}.json", m_materialsPath, name);

    if (auto properties = Material::Properties::fromFile(fullPath, fs); properties)
        return store(name, createOwningPtr<Material>(*properties));

    LOG_WARN("Could not load material config for '{}'", name);
    return nullptr;
}

Material::Properties Material::Properties::createDefault() {
    auto& textureManager = TextureManager::get();

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

        auto& textureManager = TextureManager::get();

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

}  // namespace sl
