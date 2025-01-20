#pragma once

#include <atomic>
#include <optional>

#include <starlight/core/math/Core.hh>
#include <starlight/core/Config.hh>
#include <starlight/core/Context.hh>
#include <starlight/core/memory/Memory.hh>
#include <starlight/core/event/EventHandlerSentinel.hh>
#include <starlight/renderer/Renderer.hh>
#include <starlight/renderer/camera/Camera.hh>
#include <starlight/scene/parsing/SceneParser.hh>

#include "Cameras.hh"
#include "ui/UserInterface.hh"

namespace sle {

class Application {
public:
    explicit Application(
      const sl::Config& config, std::optional<std::string> scenePath
    );

    int start();

private:
    void init();
    void initEvents();
    void startRenderLoop();
    void exit();

    std::atomic_bool m_isRunning;

    sl::Config m_config;
    sl::Context m_context;
    sl::EventProxy& m_eventProxy;
    sl::Window& m_window;
    sl::Renderer m_renderer;
    sl::EventHandlerSentinel m_eventSentinel;
    Cameras m_cameras;

    sl::Scene m_scene;
    UserInterface m_userInterface;

    sl::SceneParser m_sceneParser;
};

}  // namespace sle
