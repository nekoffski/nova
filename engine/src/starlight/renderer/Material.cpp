#include "Material.hh"

#include "starlight/core/utils/Json.hh"

namespace sl {

Material::Material(const Properties& props) :
    m_props(props), m_renderFrameNumber(0), m_textures{
        m_props.diffuseMap.get(), m_props.specularMap.get(), m_props.normalMap.get()
    } {
    LOG_TRACE("Creating Material: {}", m_props.name);
}

Material::~Material() {
    LOG_TRACE("Destroying Material - '{}'", m_props.name);

    for (const auto [shaderId, instanceId] : m_shaderInstanceIds) {
        if (auto shader = Shader::find(shaderId); shader) {
            shader->releaseInstanceResources(instanceId);
        } else {
            LOG_WARN(
              "Could release shader instance resources with id '{}', could not find shader with id '{}'",
              instanceId, shaderId
            );
        }
    }
}

bool Material::isTransparent() const {
    return m_props.diffuseMap->getProperties().isTransparent;
}

void Material::applyUniforms(
  Shader& shader, CommandBuffer& commandBuffer, u32 imageIndex,
  const u64 renderFrameNumber
) {
    if (m_renderFrameNumber != renderFrameNumber) {
        shader.setInstanceUniforms(
          commandBuffer, getShaderInstanceId(shader), imageIndex,
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

u64 Material::getShaderInstanceId(Shader& shader) {
    const auto shaderId = shader.getId();

    if (const auto record = m_shaderInstanceIds.find(shaderId);
        record != m_shaderInstanceIds.end()) [[likely]] {
        return record->second;
    }

    const auto instanceId         = shader.acquireInstanceResources(m_textures);
    m_shaderInstanceIds[shaderId] = instanceId;
    return instanceId;
}

const std::string& Material::getName() const { return m_props.name; }

const Material::Properties& Material::getProperties() const { return m_props; }

ResourceRef<Material> Material::load(const std::string& name, const FileSystem& fs) {
    return MaterialManager::get().load(name, fs);
}

ResourceRef<Material> Material::find(const std::string& name) {
    return MaterialManager::get().find(name);
}

MaterialManager::MaterialManager(const std::string& path) : m_materialsPath(path) {}

ResourceRef<Material> MaterialManager::load(
  const std::string& name, const FileSystem& fs
) {
    if (auto config = Material::Config::load(name, m_materialsPath, fs);
        not config) {
        LOG_WARN("Could not load material config for '{}'", name);
        return nullptr;
    } else {
        Material::Properties properties;

        properties.name         = config->name;
        properties.diffuseColor = config->diffuseColor;
        properties.shininess    = config->shininess;
        properties.diffuseMap =
          Texture::load(config->diffuseMap, Texture::Type::flat);
        properties.normalMap = Texture::load(config->normalMap, Texture::Type::flat);
        properties.specularMap =
          Texture::load(config->specularMap, Texture::Type::flat);

        return store(name, createOwningPtr<Material>(properties));
    }
}

Material::Config Material::Config::createDefault(const std::string& name) {
    return Material::Config{
        .name         = name,
        .diffuseColor = defaultDiffuseColor,
        .shininess    = defaultShininess,
        .diffuseMap   = defaultDiffuseMap,
        .specularMap  = defaultSpecularMap,
        .normalMap    = defaultNormalMap,
    };
}

std::optional<Material::Config> Material::Config::load(
  const std::string& name, std::string_view materialsPath, const FileSystem& fs
) {
    const auto fullPath = fmt::format("{}/{}.json", materialsPath, name);

    LOG_TRACE("Loading material config file: {}", fullPath);

    if (not fs.isFile(fullPath)) {
        LOG_ERROR("Could not find file: '{}'", fullPath);
        return {};
    }

    try {
        const auto root = kc::json::loadJson(fs.readFile(fullPath));

        return Material::Config{
            .name         = name,
            .diffuseColor = getFieldOr(root, "diffuse-color", defaultDiffuseColor),
            .shininess    = getFieldOr(root, "shininess", defaultShininess),
            .diffuseMap   = getFieldOr(root, "diffuse-map", defaultDiffuseMap),
            .specularMap  = getFieldOr(root, "specular-map", defaultSpecularMap),
            .normalMap    = getFieldOr(root, "normal-map", defaultNormalMap),
        };
    } catch (kc::json::JsonError& e) {
        LOG_ERROR("Could not parse material '{}' file: {}", name, e.asString());
    }
    return {};
}

}  // namespace sl
