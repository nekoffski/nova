#include "PointLightUI.hh"

#include <starlight/ui/UI.hh>
#include <starlight/core/event/EventProxy.hh>

#include "Events.hh"

namespace sle {

bool PointLightUI::renderSceneNode(sl::PointLight& component) {
    ImGui::BulletText(ICON_FA_LIGHTBULB "  PointLight");
    if (sl::ui::wasItemClicked()) {
        sl::EventProxy::get().emit<events::SetComponentUICallback>([&]() {
            renderInspector(component);
        });
    }
    return false;
}

void PointLightUI::renderInspector(sl::PointLight& component) {
    sl::ui::text(ICON_FA_LIGHTBULB "  PointLight");

    sl::ui::text("Position:");
    sl::ui::slider("##Position", component.position, { -10.0f, 10.0f, 0.01f });

    sl::ui::text("Attenuation:");
    auto attenuation = component.getAttenuation();
    if (sl::ui::slider("##Attenuation", attenuation, { -10.0f, 10.0f, 0.01f }))
        component.setAttenuation(attenuation);

    sl::ui::text("Color:");
    ImGui::ColorEdit4("##Color", sl::math::value_ptr(component.color));
}

}  // namespace sle
