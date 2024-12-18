#pragma once

#include <starlight/renderer/light/PointLight.hh>

#include "ComponentUI.hh"

namespace sle {

class PointLightUI : public ComponentUI<sl::PointLight> {
public:
    bool renderSceneNode(sl::PointLight& component) override;

private:
    void renderInspector(sl::PointLight& component);
};

}  // namespace sle
