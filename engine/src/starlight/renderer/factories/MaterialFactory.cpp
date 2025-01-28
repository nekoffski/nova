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

ResourceRef<Material> MaterialFactory::load(
  const std::string& name, const FileSystem& fs
) {
    const auto fullPath = fmt::format("{}/{}.json", m_materialsPath, name);

    if (auto properties = Material::Properties::fromFile(fullPath, fs); properties)
        return store(name, UniquePointer<Material>::create(*properties));

    log::warn("Could not load material config for '{}'", name);
    return nullptr;
}

}  // namespace sl
