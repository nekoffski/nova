#pragma once

#include <vector>

#include "starlight/core/Core.hh"
#include "starlight/core/window/Window.hh"
#include "starlight/renderer/camera/Camera.hh"
#include "starlight/renderer/RenderPacket.hh"

#include "Entity.hh"
#include "ComponentManager.hh"

namespace sl {

class Scene {
public:
    explicit Scene(Window& window, Camera* camera);

    RenderPacket getRenderPacket();
    void setCamera(Camera* camera);

    Entity& addEntity(std::optional<std::string> name = {});

private:
    Window& m_window;
    Camera* m_camera;

    // TODO: use StableVector instead
    ComponentManager m_componentManager;
    std::vector<Entity> m_entities;
};

}  // namespace sl
