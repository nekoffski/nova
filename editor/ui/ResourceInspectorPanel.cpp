#include "ResourceInspectorPanel.hh"

#include "starlight/scene/components/All.hh"
#include "starlight/resource/All.hh"

#include "backends/imgui_impl_vulkan.h"

#include "starlight/renderer/gpu/vulkan/VKTexture.hh"

ResourceInspectorPanel::ResourceInspectorPanel(
  sl::Scene* scene, UIState* state, Logger* logger
) : m_scene(scene), m_state(state), m_logger(logger) {}

void ResourceInspectorPanel::render() {
    if (m_state->selectedResourceId) {
        renderResourceUI(
          *m_state->selectedResourceId, *m_state->selectedResourceType
        );
    } else {
        sl::ui::text("No resource selected");
    }
}

void ResourceInspectorPanel::renderResourceUI(
  sl::u64 resourceId, ResourceType type
) {
    switch (type) {
        case ResourceType::texture:
            return renderTextureUI(sl::TextureManager::get().acquire(resourceId));
        case ResourceType::mesh:
            return renderMeshUI(sl::MeshManager::get().acquire(resourceId));
        case ResourceType::material:
            return renderMaterialUI(sl::MaterialManager::get().acquire(resourceId));
    }
}

void ResourceInspectorPanel::renderTextureUI(sl::Texture* texture) {
    sl::ui::namedScope("texture-resource-panel", [&]() {
        const auto props = texture->getProperties();
        sl::ui::text("Texture");
        sl::ui::separator();
        sl::ui::text("{}", props.name);
        sl::ui::text("Size: {}x{}", props.width, props.height);
        sl::ui::text("Channels: {}", props.channels);
        sl::ui::text("Transparent: {}", props.isTransparent);
        sl::ui::text("Writable: {}", props.isWritable);

        const auto x = ImGui::GetWindowWidth();
        m_state->getOrCreateImageHandle(texture)->show(
          { x, x }, { 0, 0 }, { 1.0f, 1.0f }
        );
    });
}

void ResourceInspectorPanel::renderMaterialUI(sl::Material* material) {
    sl::ui::namedScope("material-resource-panel", [&]() {
        const auto props = material->getProperties();
        sl::ui::text("Material");
        sl::ui::separator();
        sl::ui::text("{}", props.name);

        const auto x = ImGui::GetWindowWidth();

        sl::ui::text("Diffuse map");
        m_state->getOrCreateImageHandle(props.diffuseMap)
          ->show({ x, x }, { 0, 0 }, { 1.0f, 1.0f });

        sl::ui::text("Specular map");
        m_state->getOrCreateImageHandle(props.specularMap)
          ->show({ x, x }, { 0, 0 }, { 1.0f, 1.0f });

        sl::ui::text("Normal map");
        m_state->getOrCreateImageHandle(props.normalMap)
          ->show({ x, x }, { 0, 0 }, { 1.0f, 1.0f });
    });
}

void ResourceInspectorPanel::renderMeshUI(sl::Mesh* mesh) {}
