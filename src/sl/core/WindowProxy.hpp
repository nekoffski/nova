#pragma once

#include <memory>

#include "Window.h"

namespace sl::core {

class WindowProxy {
public:
    explicit WindowProxy(std::shared_ptr<core::Window> window)
        : m_window(window) {
    }

    const Window::Size& getSize() const {
        return m_window->getSize();
    }

private:
    std::shared_ptr<core::Window> m_window;
};
}