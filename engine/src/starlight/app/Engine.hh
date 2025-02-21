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

namespace sl {

class Engine {
public:
    explicit Engine(const Config& config);
    virtual ~Engine() = default;

    void run();

protected:
private:
    virtual void updateFrame(float frameTime) = 0;

    void initEvents();

    void render();
    float beginFrame();
    void endFrame();

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
};

}  // namespace sl
