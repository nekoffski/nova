#include "MaterialUI.hh"

#include <starlight/ui/UI.hh>
#include <starlight/core/TaskQueue.hh>

namespace sle {

MaterialUI::MaterialUI(Resources& resources) : m_resources(resources) {}

void MaterialUI::render(sl::ResourceRef<sl::Material> material) {
    sl::ui::text(ICON_FA_PRESCRIPTION_BOTTLE "  Material - {}", material.getName());
    sl::ui::separator();

    auto materialTextures = material->getTextures();
    const auto width      = ImGui::GetWindowWidth() / 1.5f;

    auto textures = sl::TextureFactory::get().getAll();

    bool textureChanged = false;

    sl::ui::text("Diffuse color:");
    ImGui::ColorEdit4(
      "##diffuse-color", sl::math::value_ptr(material->diffuseColor)
    );
    sl::ui::text("Shininess:");
    ImGui::SliderFloat("##shininess", &material->shininess, 0.0f, 64.0f);

    sl::ui::combo(
      "Diffuse map", materialTextures.diffuseMap.getName(), textures,
      [&](auto& texture) {
          materialTextures.diffuseMap = texture;
          textureChanged              = true;
      }
    );
    m_resources.getImageHandle(materialTextures.diffuseMap)
      .show({ width, width }, { 0, 0 }, { 1.0f, 1.0f });

    sl::ui::combo(
      "Specular map", materialTextures.specularMap.getName(), textures,
      [&](auto& texture) {
          materialTextures.specularMap = texture;
          textureChanged               = true;
      }
    );
    m_resources.getImageHandle(materialTextures.specularMap)
      .show({ width, width }, { 0, 0 }, { 1.0f, 1.0f });

    sl::ui::combo(
      "Normal map", materialTextures.normalMap.getName(), textures,
      [&](auto& texture) {
          materialTextures.normalMap = texture;
          textureChanged             = true;
      }
    );
    m_resources.getImageHandle(materialTextures.normalMap)
      .show({ width, width }, { 0, 0 }, { 1.0f, 1.0f });

    if (textureChanged) {
        sl::TaskQueue::get().push(
          sl::TaskQueue::Type::postFrameRender,
          [materialTextures, material]() mutable {
              material->setTextures(materialTextures);
          }
        );
    }
}

}  // namespace sle
