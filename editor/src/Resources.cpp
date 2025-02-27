#include "Resources.hh"

#include <starlight/app/factories/MaterialFactory.hh>
#include <starlight/app/factories/MeshFactory.hh>
#include <starlight/app/factories/TextureFactory.hh>

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
    // auto meshes = sl::MeshFactory::get().getAll();
    // m_meshes.insert(meshes.begin(), meshes.end());

    // auto materials = sl::MaterialFactory::get().getAll();
    // m_materials.insert(materials.begin(), materials.end());

    // auto textures = sl::TextureFactory::get().getAll();
    // m_textures.insert(textures.begin(), textures.end());
}

static auto addResource(auto& manager, auto& refSet) {
    // auto resource = manager.create();
    // refSet.insert(resource);
    // return resource;
}

sl::SharedPtr<sl::Mesh> Resources::addMesh() {
    // auto mesh = sl::MeshFactory::get().create();
    // TODO:
    return nullptr;
}

sl::SharedPtr<sl::Texture> Resources::addTexture() {
    // return addResource(sl::TextureFactory::get(), m_textures);
}

sl::SharedPtr<sl::Material> Resources::addMaterial() {
    // return addResource(sl::MaterialFactory::get(), m_materials);
}

sl::ui::ImageHandle& Resources::getImageHandle(sl::Texture* texture) {
    auto id     = texture->id;
    auto record = m_imageHandles.find(id);

    if (record != m_imageHandles.end()) return *(record->second);

    m_imageHandles[id] = sl::ui::ImageHandle::createHandle(texture);
    return *m_imageHandles.at(id);
}

}  // namespace sle
