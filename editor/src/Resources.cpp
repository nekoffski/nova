#include "Resources.hh"

namespace sle {

Resources::Resources() {
    meshes.push_back(sl::Mesh::getCube());
    meshes.push_back(sl::Mesh::getUnitSphere());
    meshes.push_back(sl::Mesh::getPlane());

    materials.push_back(sl::Material::getDefault());

    // load some default stuff
    materials.push_back(sl::Material::load("Builtin.Material.Test"));
}

}  // namespace sle
