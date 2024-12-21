#include "Resources.hh"

namespace sle {

Resources::Resources() {
    meshes.push_back(sl::MeshManager::get().getCube());
    meshes.push_back(sl::MeshManager::get().getUnitSphere());
    meshes.push_back(sl::MeshManager::get().getPlane());

    materials.push_back(sl::MaterialManager::get().getDefault());

    // load some default stuff
    auto mat = sl::MaterialManager::get().load("Builtin.Material.Test");
    materials.push_back(mat);

    // textures.push_back(mat->)
}

sl::ui::ImageHandle& Resources::getImageHandle(sl::Texture* texture) {
    auto id     = texture->getId();
    auto record = m_imageHandles.find(id);

    if (record != m_imageHandles.end()) return *(record->second);

    m_imageHandles[id] = sl::ui::ImageHandle::createHandle(texture);
    return *m_imageHandles.at(id);
}

}  // namespace sle
