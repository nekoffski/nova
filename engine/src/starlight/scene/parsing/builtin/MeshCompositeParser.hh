#pragma once

#include "starlight/scene/parsing/ComponentSerializer.hh"
#include "starlight/scene/parsing/ComponentDeserializer.hh"

#include "starlight/renderer/MeshComposite.hh"

namespace sl {

struct MeshCompositeSerializer : ComponentSerializer<MeshComposite> {
    std::string getName() const override;
    kc::json::Node serialize(MeshComposite& component) const override;
};

class MeshCompositeDeserializer : public ComponentDeserializer {
public:
    std::string getName() const override;
    void deserialize(Entity& entity, const kc::json::Node& json) const override;

private:
    ResourceRef<Mesh> getMesh(const std::string& name) const;
    ResourceRef<Material> getMaterial(const std::string& name) const;
};

}  // namespace sl