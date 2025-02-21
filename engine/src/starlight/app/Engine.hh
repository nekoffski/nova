#pragma once

#include <atomic>

#include "starlight/core/Core.hh"
#include "starlight/core/Config.hh"
#include "starlight/core/Time.hh"
#include "starlight/core/Globals.hh"
#include "starlight/core/TaskQueue.hh"
#include "starlight/event/EventBroker.hh"
#include "starlight/event/EventHandlerSentinel.hh"
#include "starlight/window/Window.hh"
#include "starlight/window/Input.hh"
#include "starlight/renderer/gpu/Device.hh"
#include "starlight/renderer/Renderer.hh"
#include "starlight/renderer/RenderGraph.hh"
#include "scene/Scene.hh"

#include "starlight/renderer/camera/Camera.hh"
#include "starlight/renderer/camera/EulerCamera.hh"

namespace sl {

class Engine {
public:
    explicit Engine(const Config& config);
    virtual ~Engine() = default;

    void run();

private:
    virtual void update(float frameTime) = 0;
    void updateFrame(float frameTime);

    void initEvents();

    void render();
    float beginFrame();
    void endFrame();

protected:
    Scene* getScene();
    RenderGraph* getRenderGraph();

private:
    Globals m_globals;
    std::atomic_bool m_isRunning;

    Clock m_clock;
    TaskQueue m_taskQueue;

    EventBroker m_eventBroker;
    EventProxy& m_eventProxy;
    EventHandlerSentinel m_eventSentinel;

    Window m_window;
    Input m_input;

    Device m_device;
    Renderer m_renderer;

    EulerCamera m_defaultCamera;
    Scene m_defaultScene;
    RenderGraph m_defaultRenderGraph;

    Camera* m_camera;
    Scene* m_scene;
    RenderGraph* m_renderGraph;
};

}  // namespace sl
