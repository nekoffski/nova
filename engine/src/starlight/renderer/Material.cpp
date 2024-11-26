#include "Material.hh"

#include "starlight/core/utils/Json.hh"

namespace sl {

Material::Material(const Properties& props) :
    m_props(props), m_renderFrameNumber(0), m_textures{
        m_props.diffuseMap.get(), m_props.specularMap.get(), m_props.normalMap.get()
    } {
    LOG_TRACE("Creating Material");
}

Material::~Material() {
    LOG_TRACE("Destroying Material");

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
    const auto fullPath = fmt::format("{}/{}.json", m_materialsPath, name);

    if (auto properties = Material::Properties::fromFile(fullPath, fs); properties)
        return store(name, createOwningPtr<Material>(*properties));

    LOG_WARN("Could not load material config for '{}'", name);
    return nullptr;
}

Material::Properties::Properties() :
    diffuseColor(Material::defaultDiffuseColor),
    diffuseMap(Texture::getDefaultDiffuseMap()),
    specularMap(Texture::getDefaultSpecularMap()),
    normalMap(Texture::getDefaultNormalMap()),
    shininess(Material::defaultShininess) {}

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

        Properties props;

        getOptField<Vec4<f32>>(root, "diffuse-color", [&](const auto& value) {
            props.diffuseColor = value;
        });
        getOptField<std::string>(root, "diffuse-map", [&](const auto& value) {
            props.diffuseMap = Texture::load(value, Texture::Type::flat);
        });
        getOptField<std::string>(root, "specular-map", [&](const auto& value) {
            props.specularMap = Texture::load(value, Texture::Type::flat);
        });
        getOptField<std::string>(root, "normal-map", [&](const auto& value) {
            props.normalMap = Texture::load(value, Texture::Type::flat);
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
