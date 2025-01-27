#include "GLFWWindow.hh"

#include <GLFW/glfw3.h>

#include "starlight/core/Log.hh"
#include "starlight/core/event/Input.hh"

namespace sl::glfw {

#define GLFW_WINDOW_PTR(ptr) static_cast<GLFWwindow*>(ptr)
#define GET_WINDOW_DATA(window) \
    static_cast<GLFWWindow::Data*>(glfwGetWindowUserPointer(GLFW_WINDOW_PTR(window)))

static KeyAction glfwToNovaKeyAction(int action) {
    switch (action) {
        case GLFW_PRESS:
            return KeyAction::press;

        case GLFW_REPEAT:
            return KeyAction::repeat;

        case GLFW_RELEASE:
            return KeyAction::release;
    }

    return KeyAction::unknown;
}

static MouseAction glfwToNovaMouseAction(int action) {
    switch (action) {
        case GLFW_PRESS:
            return MouseAction::press;

        case GLFW_RELEASE:
            return MouseAction::release;
    }

    return MouseAction::unknown;
}

GLFWWindow::GLFWWindow(const Config::Window& config) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_windowHandle = glfwCreateWindow(
      config.width, config.height, config.name.c_str(), nullptr, nullptr
    );

    glfwMakeContextCurrent(GLFW_WINDOW_PTR(m_windowHandle));
    glfwSetWindowUserPointer(GLFW_WINDOW_PTR(m_windowHandle), &m_windowData);

    auto data        = GET_WINDOW_DATA(m_windowHandle);
    data->lastWidth  = static_cast<i32>(config.width);
    data->lastHeight = static_cast<i32>(config.height);
}

bool GLFWWindow::isKeyPressed(WindowImpl::Key keyCode) const {
    return glfwGetKey(GLFW_WINDOW_PTR(m_windowHandle), keyCode) == GLFW_PRESS;
}

bool GLFWWindow::isMouseButtonPressed(WindowImpl::Button buttonCode) const {
    return glfwGetMouseButton(GLFW_WINDOW_PTR(m_windowHandle), buttonCode)
           == GLFW_PRESS;
}

void GLFWWindow::onKeyCallback(OnKeyCallback callback) {
    m_windowData.onKey = callback;

    static auto onKeyCallback =
      [](
        GLFWwindow* window, int key, [[maybe_unused]] int, int action,
        [[maybe_unused]] int
      ) { GET_WINDOW_DATA(window)->onKey(glfwToNovaKeyAction(action), key); };

    glfwSetKeyCallback(GLFW_WINDOW_PTR(m_windowHandle), onKeyCallback);
}

void GLFWWindow::onMouseCallback(OnMouseCallback callback) {
    m_windowData.onMouse = callback;

    static auto onMouseButtonCallback =
      [](GLFWwindow* window, int button, int action, [[maybe_unused]] int) {
          GET_WINDOW_DATA(window)->onMouse(glfwToNovaMouseAction(action), button);
      };

    glfwSetMouseButtonCallback(
      GLFW_WINDOW_PTR(m_windowHandle), onMouseButtonCallback
    );
}

void GLFWWindow::onScrollCallback(OnScrollCallback callback) {
    m_windowData.onScroll = callback;

    static auto onScrollCallback =
      [](GLFWwindow* window, [[maybe_unused]] double xOffset, double yOffset) {
          log::trace("Scroll: {}/{}", xOffset, yOffset);
          GET_WINDOW_DATA(window)->onScroll(static_cast<float>(yOffset));
      };

    glfwSetScrollCallback(GLFW_WINDOW_PTR(m_windowHandle), onScrollCallback);
}

void GLFWWindow::onWindowCloseCallback(OnWindowCloseCallback callback) {
    m_windowData.onWindowClose = callback;

    static auto onWindowCloseCallback = [](GLFWwindow* window) {
        GET_WINDOW_DATA(window)->onWindowClose();
    };

    glfwSetWindowCloseCallback(
      GLFW_WINDOW_PTR(m_windowHandle), onWindowCloseCallback
    );
}

void GLFWWindow::onWindowResizeCallback(OnWindowResizeCallback callback) {
    m_windowData.onWindowResize = callback;

    static auto onWindowResizeCallback =
      [](GLFWwindow* window, int width, int height) {
          auto data = GET_WINDOW_DATA(window);

          if (data->lastWidth != width || data->lastHeight != height) {
              data->lastWidth  = width;
              data->lastHeight = height;
              data->onWindowResize(
                static_cast<float>(width), static_cast<float>(height)
              );
          }
      };

    glfwSetWindowSizeCallback(
      GLFW_WINDOW_PTR(m_windowHandle), onWindowResizeCallback
    );
}

void GLFWWindow::update() { glfwPollEvents(); }

void GLFWWindow::hideCursor() {
    glfwSetInputMode(
      GLFW_WINDOW_PTR(m_windowHandle), GLFW_CURSOR, GLFW_CURSOR_DISABLED
    );
}

void GLFWWindow::showCursor() {
    glfwSetInputMode(
      GLFW_WINDOW_PTR(m_windowHandle), GLFW_CURSOR, GLFW_CURSOR_NORMAL
    );
}

void GLFWWindow::swapBuffers() { glfwSwapBuffers(GLFW_WINDOW_PTR(m_windowHandle)); }

std::string_view GLFWWindow::getVendor() const { return "GLFW3"; }

Vec2<u32> GLFWWindow::getFramebufferSize() const {
    int width, height;
    glfwGetFramebufferSize(GLFW_WINDOW_PTR(m_windowHandle), &width, &height);

    return Vec2<u32>{ static_cast<u32>(width), static_cast<u32>(height) };
}

Vec2<u32> GLFWWindow::getSize() const {
    int width, height;
    glfwGetWindowSize(GLFW_WINDOW_PTR(m_windowHandle), &width, &height);

    return Vec2<u32>{ static_cast<u32>(width), static_cast<u32>(height) };
}

Vec2<f32> GLFWWindow::getMousePosition() const {
    double x, y;
    glfwGetCursorPos(GLFW_WINDOW_PTR(m_windowHandle), &x, &y);

    return Vec2<f32>{ x, y };
}

void* GLFWWindow::getHandle() { return m_windowHandle; }

}  // namespace sl::glfw