#pragma once

#include <vector>

#include "starlight/core/Core.hh"
#include "starlight/core/window/Window.hh"
#include "starlight/renderer/camera/Camera.hh"
#include "starlight/renderer/RenderPacket.hh"
#include "starlight/renderer/Skybox.hh"

#include "Entity.hh"
#include "ComponentManager.hh"

namespace sl {

class Scene {
public:
    explicit Scene(Window& window, Camera* camera);

    RenderPacket getRenderPacket();

    void setCamera(Camera& camera);
    void setSkybox(Skybox& skybox);

    Entity& addEntity(std::optional<std::string> name = {});

private:
    Window& m_window;
    Camera* m_camera;
    Skybox* m_skybox;

    // TODO: use StableVector instead
    ComponentManager m_componentManager;
    std::vector<Entity> m_entities;
};

}  // namespace sl
