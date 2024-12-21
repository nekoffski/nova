#include "MeshCompositeParser.hh"

namespace sl {

std::string MeshCompositeSerializer::getName() const { return "MeshComposite"; }

kc::json::Node MeshCompositeSerializer::serialize(MeshComposite& component) const {
    kc::json::Node json;

    // just root for now
    auto& root = component.getRoot();

    json["material"] = root.material.getName();
    json["mesh"]     = root.mesh.getName();

    return json;
}

std::string MeshCompositeDeserializer::getName() const { return "MeshComposite"; }

void MeshCompositeDeserializer::deserialize(
  Entity& entity, const kc::json::Node& json
) const {
    auto mesh     = getMesh(json["mesh"].as<std::string>());
    auto material = getMaterial(json["material"].as<std::string>());

    entity.addComponent<MeshComposite>(mesh, material);
}

// TODO: store default materials/meshes/shaders/textures in some lookup table
ResourceRef<Mesh> MeshCompositeDeserializer::getMesh(const std::string& name) const {
    return MeshManager::get().find(name);
}

ResourceRef<Material> MeshCompositeDeserializer::getMaterial(const std::string& name
) const {
    return MaterialManager::get().find(name);
}

}  // namespace sl
