#include "Resources.hh"

#include <starlight/renderer/factories/MaterialFactory.hh>
#include <starlight/renderer/factories/MeshFactory.hh>
#include <starlight/renderer/factories/TextureFactory.hh>

namespace sle {

Resources::Resources() {
    m_materials.insert(sl::MaterialFactory::get().load("Builtin.Material.Test"));
    sync();

    // materials.push_back(sl::MaterialFactory::get().getDefault());

    // // load some default stuff

    // auto mat = sl::MaterialFactory::get().load("Builtin.Material.Test");
    // materials.push_back(mat);

    // textures.push_back(mat->)
}

void Resources::sync() {
    auto meshes = sl::MeshFactory::get().getAll();
    m_meshes.insert(meshes.begin(), meshes.end());

    auto materials = sl::MaterialFactory::get().getAll();
    m_materials.insert(materials.begin(), materials.end());

    auto textures = sl::TextureFactory::get().getAll();
    m_textures.insert(textures.begin(), textures.end());
}

static auto addResource(auto& manager, auto& refSet) {
    auto resource = manager.create();
    refSet.insert(resource);
    return resource;
}

sl::ResourceRef<sl::Mesh> Resources::addMesh() {
    // auto mesh = sl::MeshFactory::get().create();
    // TODO:
    return nullptr;
}

sl::ResourceRef<sl::Texture> Resources::addTexture() {
    return addResource(sl::TextureFactory::get(), m_textures);
}

sl::ResourceRef<sl::Material> Resources::addMaterial() {
    return addResource(sl::MaterialFactory::get(), m_materials);
}

sl::ui::ImageHandle& Resources::getImageHandle(sl::Texture* texture) {
    auto id     = texture->getId();
    auto record = m_imageHandles.find(id);

    if (record != m_imageHandles.end()) return *(record->second);

    m_imageHandles[id] = sl::ui::ImageHandle::createHandle(texture);
    return *m_imageHandles.at(id);
}

}  // namespace sle
