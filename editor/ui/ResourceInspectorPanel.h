#pragma once

#pragma once

#include "starlight/scene/Scene.h"

#include "Core.h"
#include "Console.h"

#include "starlight/renderer/gpu/Texture.h"
#include "starlight/renderer/gpu/Mesh.h"
#include "starlight/renderer/Material.h"

class ResourceInspectorPanel {
public:
    explicit ResourceInspectorPanel(
      sl::Scene* scene, UIState* state, Logger* logger
    );

    void render();

private:
    void renderResourceUI(sl::u64 resourceId, ResourceType type);
    void renderTextureUI(sl::Texture* texture);
    void renderMaterialUI(sl::Material* material);
    void renderMeshUI(sl::Mesh* mesh);

    sl::Scene* m_scene;
    UIState* m_state;
    Logger* m_logger;
};
