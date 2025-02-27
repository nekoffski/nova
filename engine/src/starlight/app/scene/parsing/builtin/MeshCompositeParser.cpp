#include "MeshCompositeParser.hh"

#include "starlight/app/factories/MeshFactory.hh"
#include "starlight/app/factories/MaterialFactory.hh"

namespace sl {

std::string MeshCompositeSerializer::getName() const { return "MeshComposite"; }

nlohmann::json MeshCompositeSerializer::serialize(MeshComposite& component) const {
    nlohmann::json json;

    // just root for now
    auto& root = component.getRoot();

    json["material"] = root.material->name;
    json["mesh"]     = root.mesh->name;

    return json;
}

std::string MeshCompositeDeserializer::getName() const { return "MeshComposite"; }

void MeshCompositeDeserializer::deserialize(
  Entity& entity, const nlohmann::json& json
) const {
    auto mesh     = getMesh(json.at("mesh").get<std::string>());
    auto material = getMaterial(json.at("material").get<std::string>());

    entity.addComponent<MeshComposite>(mesh, material);
}

// TODO: store default materials/meshes/shaders/textures in some lookup table
SharedPtr<Mesh> MeshCompositeDeserializer::getMesh(const std::string& name) const {
    return MeshFactory::get().find(name);
}

SharedPtr<Material> MeshCompositeDeserializer::getMaterial(const std::string& name
) const {
    return MaterialFactory::get().find(name);
}

}  // namespace sl
