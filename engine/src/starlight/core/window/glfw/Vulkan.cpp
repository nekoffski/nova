#include "Vulkan.hh"

#ifdef SL_USE_GLFW
#ifdef SL_USE_VK

#include <vector>

#include <GLFW/glfw3.h>

#include "starlight/core/Log.hh"

namespace sl::glfw {

VkSurfaceKHR createVulkanSurface(
  VkInstance instance, void* windowHandle, VkAllocationCallbacks* allocator
) {
    VkSurfaceKHR surface;

    log::trace("Creating Vulkan surface");
    log::expect(windowHandle != nullptr, "windowHandle==nullptr");

    auto result = glfwCreateWindowSurface(
      instance, static_cast<GLFWwindow*>(windowHandle), allocator, &surface
    );
    log::expect(
      result == VK_SUCCESS, "GLFW Vulkan Surface creating error: {}",
      fmt::underlying(result)
    );

    return surface;
}

std::vector<const char*> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount
    );

    return std::vector<const char*>{
        glfwExtensions, glfwExtensions + glfwExtensionCount
    };
}

}  // namespace sl::glfw

#endif
#endif
