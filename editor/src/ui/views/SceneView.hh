#pragma once

#include <unordered_map>
#include <typeindex>

#include <starlight/ui/UI.hh>
#include <starlight/app/scene/Scene.hh>

#include "Console.hh"
#include "Resources.hh"
#include "components/ComponentUI.hh"

namespace sle {

class SceneView {
    class SceneTab {
    public:
        explicit SceneTab(sl::Scene* scene);

    protected:
        sl::Scene* m_scene;
    };

    class EntitiesTab : public SceneTab {
        using ComponentUIs =
          std::unordered_map<std::type_index, sl::UniquePointer<ComponentUIBase>>;

    public:
        explicit EntitiesTab(sl::Scene* scene, Resources& resources);
        void render();

    private:
        void selectEntity(sl::Entity& entity, bool clearComponentCallback);

        template <typename T, typename... Args>
        requires(std::is_base_of_v<ComponentUIBase, T> && std::is_constructible_v<T, Args...>)
        void addComponentUI(Args&&... args) {
            auto ui = sl::UniquePointer<T>::create(std::forward<Args>(args)...);
            const auto index      = ui->getTypeIndex();
            m_componentUIs[index] = std::move(ui);
        }

        ComponentUIs m_componentUIs;
        sl::Entity* m_selectedEntity;
    };

    class SkyboxTab : public SceneTab {
    public:
        using SceneTab::SceneTab;
        void render();
    };

    class CameraTab : public SceneTab {
    public:
        using SceneTab::SceneTab;
        void render();
    };

public:
    explicit SceneView(sl::Scene* scene, Resources& resources);

    void render();

private:
    sl::ui::TabMenu m_tabMenu;

    EntitiesTab m_entitiesTab;
    SkyboxTab m_skyboxTab;
    CameraTab m_cameraTab;
};

}  // namespace sle
