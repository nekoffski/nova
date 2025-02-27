#include "TextureUI.hh"

#include <starlight/ui/UI.hh>

namespace sle {

void TextureUI::render(sl::SharedPtr<sl::Texture> texture) {
    sl::ui::text(ICON_FA_PRESCRIPTION_BOTTLE "  Texture - {}", texture->name);
    sl::ui::separator();

    // auto props       = material->getProperties();
    // const auto width = ImGui::GetWindowWidth() / 1.5f;

    // sl::ui::text("Diffuse map:");
    // m_resources.getImageHandle(props.diffuseMap)
    //   .show({ width, width }, { 0, 0 }, { 1.0f, 1.0f });

    // sl::ui::text("Specular map:");
    // m_resources.getImageHandle(props.specularMap)
    //   .show({ width, width }, { 0, 0 }, { 1.0f, 1.0f });

    // sl::ui::text("Normal map:");
    // m_resources.getImageHandle(props.normalMap)
    //   .show({ width, width }, { 0, 0 }, { 1.0f, 1.0f });
}

}  // namespace sle
