#pragma once

#include <atomic>
#include <optional>

#include <starlight/core/math/Core.hh>
#include <starlight/core/Config.hh>
#include <starlight/core/memory/Memory.hh>
#include <starlight/event/EventHandlerSentinel.hh>
#include <starlight/renderer/Renderer.hh>
#include <starlight/renderer/camera/Camera.hh>
#include <starlight/app/scene/parsing/SceneParser.hh>
#include <starlight/app/Engine.hh>

#include "Cameras.hh"
#include "ui/UserInterface.hh"

namespace sle {

class Application : public sl::Engine {
public:
    explicit Application(
      const sl::Config& config, std::optional<std::string> scenePath
    );

private:
    void update(float frameTime) override;

    void initEvents();

    sl::EventHandlerSentinel m_eventSentinel;
    Cameras m_cameras;

    UserInterface m_userInterface;
    sl::SceneParser m_sceneParser;
};

}  // namespace sle
