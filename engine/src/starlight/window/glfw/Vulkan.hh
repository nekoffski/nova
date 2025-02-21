#pragma once
#ifdef SL_USE_GLFW
#ifdef SL_USE_VK

#include <vector>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace sl::glfw {

VkSurfaceKHR createVulkanSurface(
  VkInstance instance, void* windowHandle, VkAllocationCallbacks* allocator
);

std::vector<const char*> getRequiredExtensions();

}  // namespace sl::glfw

#endif
#endif
