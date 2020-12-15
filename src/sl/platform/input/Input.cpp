#include "Input.h"

#include "sl/core/log/Logger.h"
#include "sl/platform/input/glfw/GLFWInput.h"

namespace sl::platform::input {

std::shared_ptr<Input> Input::create(misc::types::NotNullPtr<void> windowHandle) {
#ifdef STARL_USE_GLFW
    SL_INFO("instancing glfw input");
    return std::make_shared<glfw::GLFWInput>(windowHandle);
#endif
}
} // namespace sl::platform::input