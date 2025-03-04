#include "MaterialUI.hh"

#include <starlight/ui/UI.hh>
#include <starlight/core/TaskQueue.hh>
#include <starlight/app/factories/TextureFactory.hh>

namespace sle {

MaterialUI::MaterialUI(Resources& resources) : m_resources(resources) {}

void MaterialUI::render(sl::SharedPtr<sl::Material> material) {
    sl::ui::text(ICON_FA_PRESCRIPTION_BOTTLE "  Material - {}", material->name);
    sl::ui::separator();

    // auto materialTextures = material->textures;
    // const auto width      = ImGui::GetWindowWidth() / 1.5f;

    // auto textures = sl::TextureFactory::get().getAll();

    // bool textureChanged = false;

    // sl::ui::text("Diffuse color:");
    // ImGui::ColorEdit4(
    //   "##diffuse-color", sl::math::value_ptr(material->diffuseColor)
    // );
    // sl::ui::text("Shininess:");
    // ImGui::SliderFloat("##shininess", &material->shininess, 0.0f, 64.0f);

    // sl::ui::combo(
    //   "Diffuse map", materialTextures.diffuse.name, textures,
    //   [&](auto& texture) {
    //       materialTextures.diffuse = texture;
    //       textureChanged           = true;
    //   }
    // );
    // m_resources.getImageHandle(materialTextures.diffuse)
    //   .show({ width, width }, { 0, 0 }, { 1.0f, 1.0f });

    // sl::ui::combo(
    //   "Specular map", materialTextures.specular.name, textures,
    //   [&](auto& texture) {
    //       materialTextures.specular = texture;
    //       textureChanged            = true;
    //   }
    // );
    // m_resources.getImageHandle(materialTextures.specular)
    //   .show({ width, width }, { 0, 0 }, { 1.0f, 1.0f });

    // sl::ui::combo(
    //   "Normal map", materialTextures.normal.name, textures,
    //   [&](auto& texture) {
    //       materialTextures.normal = texture;
    //       textureChanged          = true;
    //   }
    // );
    // m_resources.getImageHandle(materialTextures.normal)
    //   .show({ width, width }, { 0, 0 }, { 1.0f, 1.0f });

    // if (textureChanged) {
    //     sl::TaskQueue::get().callPostFrame([materialTextures, material]() mutable
    //     {
    //         material->textures = materialTextures;
    //     });
    // }
}

}  // namespace sle
