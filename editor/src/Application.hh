#pragma once

#include <atomic>

#include <starlight/core/math/Core.hh>
#include <starlight/core/Config.hh>
#include <starlight/core/Context.hh>
#include <starlight/core/event/EventHandlerSentinel.hh>
#include <starlight/renderer/RendererFrontend.hh>
#include <starlight/renderer/camera/Camera.hh>
#include <starlight/core/memory/Memory.hh>

#include "Cameras.hh"
#include "ui/UserInterface.hh"

namespace sle {

class Application {
public:
    explicit Application(const sl::Config& config);

    int start();

private:
    void init();
    void initEvents();
    void startRenderLoop();
    void exit();

    void onViewportResize(const sl::Vec2<sl::u32>& viewport);

    std::atomic_bool m_isRunning;

    sl::Config m_config;
    sl::Context m_context;
    sl::Window& m_window;
    sl::RendererFrontend m_renderer;
    sl::EventHandlerSentinel m_eventSentinel;
    Cameras m_cameras;

    sl::Scene m_scene;
    UserInterface m_userInterface;
};

}  // namespace sle
