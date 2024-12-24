#include "Resources.hh"

namespace sle {

Resources::Resources() {
    m_materials.insert(sl::MaterialManager::get().load("Builtin.Material.Test"));
    sync();

    // materials.push_back(sl::MaterialManager::get().getDefault());

    // // load some default stuff

    // auto mat = sl::MaterialManager::get().load("Builtin.Material.Test");
    // materials.push_back(mat);

    // textures.push_back(mat->)
}

void Resources::sync() {
    auto meshes = sl::MeshManager::get().getAll();
    m_meshes.insert(meshes.begin(), meshes.end());

    auto materials = sl::MaterialManager::get().getAll();
    m_materials.insert(materials.begin(), materials.end());

    auto textures = sl::TextureManager::get().getAll();
    m_textures.insert(textures.begin(), textures.end());
}

static auto addResource(auto& manager, auto& refSet) {
    auto resource = manager.create();
    refSet.insert(resource);
    return resource;
}

sl::ResourceRef<sl::Mesh> Resources::addMesh() {
    // auto mesh = sl::MeshManager::get().create();
    // TODO:
    return nullptr;
}

sl::ResourceRef<sl::Texture> Resources::addTexture() {
    return addResource(sl::TextureManager::get(), m_textures);
}

sl::ResourceRef<sl::Material> Resources::addMaterial() {
    return addResource(sl::MaterialManager::get(), m_materials);
}

sl::ui::ImageHandle& Resources::getImageHandle(sl::Texture* texture) {
    auto id     = texture->getId();
    auto record = m_imageHandles.find(id);

    if (record != m_imageHandles.end()) return *(record->second);

    m_imageHandles[id] = sl::ui::ImageHandle::createHandle(texture);
    return *m_imageHandles.at(id);
}

}  // namespace sle
