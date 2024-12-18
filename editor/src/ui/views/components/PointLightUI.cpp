#include "PointLightUI.hh"

#include <starlight/ui/UI.hh>
#include <starlight/core/event/EventProxy.hh>

#include "Events.hh"

namespace sle {

bool PointLightUI::renderSceneNode(sl::PointLight& component) {
    ImGui::BulletText(ICON_FA_LIGHTBULB "  PointLight");
    if (sl::ui::wasItemClicked()) {
        sl::EventProxy::get().emit<events::SetComponentCallback>([&]() {
            renderInspector(component);
        });
    }

    return false;
}

void PointLightUI::renderInspector(sl::PointLight& component) {
    sl::ui::text(ICON_FA_LIGHTBULB "  PointLight");
    sl::ui::text("Position:");
    sl::ui::slider("##Position", component.data.position, { -10.0f, 10.0f, 0.01f });
    sl::ui::text("Attenuation:");
    if (sl::ui::slider(
          "##Attenuation", component.data.attenuationFactors,
          { -10.0f, 10.0f, 0.01f }
        )) {
        component.generateLODs();
    }
    sl::ui::text("Color:");
    ImGui::ColorEdit4("##Color", sl::math::value_ptr(component.data.color));
}

}  // namespace sle
