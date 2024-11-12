#pragma once

#pragma once

#include "starlight/scene/Scene.hh"

#include "Core.hh"
#include "Console.hh"

#include "starlight/renderer/gpu/Texture.hh"

class EntityInspectorPanel {
    struct EntityData {
        int selectedMeshIndex     = 0;
        int selectedMaterialIndex = 0;
    };

public:
    explicit EntityInspectorPanel(sl::Scene* scene, UIState* state, Logger* logger);

    void render();

private:
    void renderEntityUI(sl::u64 entityId);
    void renderResourceUI(sl::u64 resourceId, ResourceType type);

    void renderTextureUI(sl::Texture* texture);

    sl::Scene* m_scene;
    UIState* m_state;
    Logger* m_logger;

    sl::ui::Slider3 m_translationSlider;
    sl::ui::Slider3 m_scaleSlider;
    sl::ui::Slider3 m_orientationSlider;

    int m_selectedComponentIndex;

    std::unordered_map<sl::u64, EntityData> m_entitiesData;
};
