#pragma once

#include <atomic>

#include <starlight/core/Config.hh>
#include <starlight/core/Context.hh>
#include <starlight/core/event/EventHandlerSentinel.hh>
#include <starlight/renderer/RendererFrontend.hh>

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

    void onFrame(float deltaTime);

    std::atomic_bool m_isRunning;

    sl::Context m_context;
    sl::EventHandlerSentinel m_eventSentinel;
    sl::RendererFrontend m_renderer;
};

}  // namespace sle
