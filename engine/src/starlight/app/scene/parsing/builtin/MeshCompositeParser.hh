#pragma once

#include "starlight/app/scene/parsing/ComponentSerializer.hh"
#include "starlight/app/scene/parsing/ComponentDeserializer.hh"

#include "starlight/renderer/MeshComposite.hh"

namespace sl {

struct MeshCompositeSerializer : ComponentSerializer<MeshComposite> {
    std::string getName() const override;
    nlohmann::json serialize(MeshComposite& component) const override;
};

class MeshCompositeDeserializer : public ComponentDeserializer {
public:
    std::string getName() const override;
    void deserialize(Entity& entity, const nlohmann::json& json) const override;

private:
    SharedPointer<Mesh> getMesh(const std::string& name) const;
    SharedPointer<Material> getMaterial(const std::string& name) const;
};

}  // namespace sl
