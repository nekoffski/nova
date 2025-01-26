#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <kc/core/Log.h>
#include <kc/core/ErrorBase.hpp>

#include "starlight/core/Core.hh"

#define VK_ASSERT(expr) \
    { ASSERT(expr == VK_SUCCESS, "Vulkan Fatal Error: {}", fmt::underlying(expr)); }

namespace sl::vk {

std::string getResultString(VkResult result, bool extended);

bool isGood(VkResult);

using Allocator = VkAllocationCallbacks;

}  // namespace sl::vk
