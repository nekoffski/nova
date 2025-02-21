#include "MaterialFactory.hh"

#include "starlight/renderer/factories/TextureFactory.hh"

namespace sl {

MaterialFactory::MaterialFactory(const std::string& path) :
    ResourceFactory("Material"), m_materialsPath(path),
    m_defaultMaterial(create("DefaultMaterial")) {}

ResourceRef<Material> MaterialFactory::getDefault() { return m_defaultMaterial; }

ResourceRef<Material> MaterialFactory::create(
  const std::string& name, const Material::Properties& properties
) {
    return store(name, UniquePointer<Material>::create(properties));
}

ResourceRef<Material> MaterialFactory::create(const Material::Properties& properties
) {
    return store(UniquePointer<Material>::create(properties));
}

std::optional<Material::Properties> loadProperties(
  const std::string& path, const FileSystem& fs
) {
    log::trace("Loading material properties file: {}", path);

    if (not fs.isFile(path)) {
        log::error("Could not find file: '{}'", path);
        return {};
    }

    try {
        const auto root = nlohmann::json::parse(fs.readFile(path));
        auto props      = Material::Properties::createDefault();

        json::getIfExists(root, "diffuse-color", props.diffuseColor);
        json::getIfExists(root, "diffuse-map", props.textures.diffuse);
        json::getIfExists(root, "specular-map", props.textures.specular);
        json::getIfExists(root, "normal-map", props.textures.normal);
        json::getIfExists(root, "shininess", props.shininess);

        return props;
    } catch (const nlohmann::json::parse_error& e) {
        log::error("Could not parse material '{}' file: {}", path, e.what());
    }
    return {};
}

ResourceRef<Material> MaterialFactory::load(
  const std::string& name, const FileSystem& fs
) {
    const auto fullPath = fmt::format("{}/{}.json", m_materialsPath, name);

    if (auto properties = loadProperties(fullPath, fs); properties)
        return store(name, UniquePointer<Material>::create(*properties));

    log::warn("Could not load material config for '{}'", name);
    return nullptr;
}

}  // namespace sl
