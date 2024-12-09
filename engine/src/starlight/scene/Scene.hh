#pragma once

#include <vector>

#include "starlight/core/Core.hh"
#include "starlight/core/window/Window.hh"
#include "starlight/core/utils/FlatMap.hh"
#include "starlight/renderer/camera/Camera.hh"
#include "starlight/renderer/RenderPacket.hh"
#include "starlight/renderer/Skybox.hh"

#include "Entity.hh"
#include "ComponentManager.hh"

namespace sl {

class Scene {
    static constexpr u64 maxEntities = 1024;

public:
    explicit Scene(Window& window, Camera* camera);

    RenderPacket getRenderPacket();

    void setCamera(Camera& camera);
    void setSkybox(Skybox& skybox);

    template <typename C>
    requires Callable<C, void, Entity&>
    void forEachEntity(C&& callback) {
        m_entities.forEach(std::move(callback));
    }

    Entity& addEntity(std::optional<std::string> name = {});

private:
    Window& m_window;
    Camera* m_camera;
    Skybox* m_skybox;

    ComponentManager m_componentManager;
    StableVector<Entity> m_entities;
};

}  // namespace sl
