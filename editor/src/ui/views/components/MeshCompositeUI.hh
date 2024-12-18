#pragma once

#include <starlight/renderer/MeshComposite.hh>

#include "ComponentUI.hh"
#include "Resources.hh"

namespace sle {

class MeshCompositeUI : public ComponentUI<sl::MeshComposite> {
public:
    explicit MeshCompositeUI(Resources& resources);

    bool renderSceneNode(sl::MeshComposite& component) override;

private:
    void renderInstanceUI(sl::Transform& instance);
    void renderNodeUI(sl::MeshComposite::Node& node);
    void renderMeshCompositeUI(sl::MeshComposite& mesh);

    Resources& m_resources;
};

}  // namespace sle
