#include "DirectionalLightUI.hh"

#include <starlight/ui/UI.hh>
#include <starlight/event/EventProxy.hh>

#include "Events.hh"

namespace sle {

bool DirectionalLightUI::renderSceneNode(sl::DirectionalLight& component) {
    ImGui::BulletText(ICON_FA_SUN "  DirectionalLight");

    if (sl::ui::wasItemClicked()) {
        sl::EventProxy::get().emit<events::SetComponentUICallback>([&]() {
            renderInspector(component);
        });
        return true;
    }

    return false;
}

void DirectionalLightUI::renderInspector(sl::DirectionalLight& component) {
    sl::ui::text(ICON_FA_SUN "  DirectionalLight");
    sl::ui::separator();
    sl::ui::text("Direction:");
    sl::ui::slider("##Direction", component.direction, { -1.0f, 1.0f, 0.02f });
    sl::ui::text("Color:");
    ImGui::ColorEdit4("##Color", sl::math::value_ptr(component.color));
}

}  // namespace sle
