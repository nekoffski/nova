#include "MaterialFactory.hh"

#include "starlight/core/Json.hh"
#include "starlight/core/Globals.hh"

#include "TextureFactory.hh"

namespace sl {

static Material::Properties getDefaultProperties() {
    auto& textureFactory = TextureFactory::get();
    return Material::Properties{
        .diffuseMap   = textureFactory.getDefaultDiffuseMap(),
        .specularMap  = textureFactory.getDefaultSpecularMap(),
        .normalMap    = textureFactory.getDefaultNormalMap(),
        .diffuseColor = MaterialFactory::defaultDiffuseColor,
        .shininess    = MaterialFactory::defaultShininess
    };
}

static std::optional<Material::Properties> loadProperties(
  const std::string& path, const FileSystem& fs
) {
    log::trace("Loading material properties file: {}", path);

    if (not fs.isFile(path)) {
        log::error("Could not find file: '{}'", path);
        return {};
    }

    try {
        const auto root = nlohmann::json::parse(fs.readFile(path));
        auto props      = getDefaultProperties();

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

MaterialFactory::MaterialFactory() { createDefault(); }

SharedPtr<Material> MaterialFactory::load(
  const std::string& name, const FileSystem& fs
) {
    const auto& materialsPath = Globals::get().getConfig().paths.materials;
    const auto fullPath       = fmt::format("{}/{}.json", materialsPath, name);

    if (auto props = loadProperties(fullPath, fs); props) {
        return create(name, *props);
    }

    log::warn("Could not load material: '{}'", name);
    return nullptr;
}

SharedPtr<Material> MaterialFactory::create(
  const std::string& name, const Material::Properties& properties
) {
    return save(SharedPtr<Material>::create(properties, name));
}

SharedPtr<Material> MaterialFactory::getDefault() { return m_defaultMaterial; }

void MaterialFactory::createDefault() {
    m_defaultMaterial =
      save(SharedPtr<Material>::create(getDefaultProperties(), "Material.Default"));
}

}  // namespace sl