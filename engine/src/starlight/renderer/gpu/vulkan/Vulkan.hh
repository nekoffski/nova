#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include "starlight/core/Core.hh"
#include "starlight/core/Log.hh"

namespace sl {
namespace vk {

std::string getResultString(VkResult result, bool extended);

using Allocator = VkAllocationCallbacks;

}  // namespace vk

namespace log {
constexpr void expect(VkResult result) {
    log::expect(
      result == VK_SUCCESS, "Vulkan call failed: {}",
      vk::getResultString(result, true)
    );
}
}  // namespace log

}  // namespace sl
