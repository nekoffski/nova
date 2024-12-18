#pragma once

#include <starlight/renderer/MeshComposite.hh>

#include "ComponentUI.hh"

namespace sle {

class MeshCompositeUI : public ComponentUI<sl::MeshComposite> {
public:
    bool renderSceneNode(sl::MeshComposite& component) override;
};

}  // namespace sle
