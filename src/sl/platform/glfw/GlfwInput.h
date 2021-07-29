#pragma once

#include "sl/core/Input.h"
#include "sl/core/NotNullPtr.hpp"

namespace sl::platform::glfw {

class GlfwInput : public core::Input {
public:
    explicit GlfwInput(void* windowHandle);

    bool isKeyPressed(int) const override;

    bool isMouseButtonPressed(int) const override;

    std::pair<double, double> getMousePosition() const override;

    void setOnMousePositionChange(sl::core::Mouse::Callback) override;
    void setOnMouseScroll(sl::core::Mouse::Callback) override;

private:
    void* m_windowHandle;
};

}