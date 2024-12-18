#pragma once

#include <starlight/renderer/light/DirectionalLight.hh>

#include "ComponentUI.hh"

namespace sle {

class DirectionalLightUI : public ComponentUI<sl::DirectionalLight> {
public:
    bool renderSceneNode(sl::DirectionalLight& component) override;

private:
    void renderInspector(sl::DirectionalLight& component);
};

}  // namespace sle
