#pragma once

#include <string>

#include "window/WindowImpl.hh"
#include "window/Window.hh"
#include "window/Vendor.hh"
#include "window/Input.hh"

#include "event/EventBroker.hh"
#include "Clock.hh"
#include "Config.hh"
#include "Core.hh"
#include "TaskQueue.hh"

namespace sl {

class Context : public NonCopyable {
public:
    explicit Context(const Config& config);

    template <typename C>
    requires Callable<C, void, float>
    void beginFrame(C&& callback) {
        callback(beginFrame());
        endFrame();
    }

    Window& getWindow();
    EventProxy& getEventProxy();
    const Config& getConfig() const;

private:
    float beginFrame();
    void endFrame();

    Config m_config;
    EventBroker m_eventBroker;

    WindowVendor m_windowImpl;
    Window m_window;
    Input m_input;
    Clock m_clock;
    TaskQueue m_taskQueue;
};

}  // namespace sl