// #pragma once

// #include <vector>

// #include "starlight/core/Core.hh"

// #include "VKLogicalDevice.hh"
// #include "VKContext.hh"

// #include "Vulkan.hh"
// #include "fwd.hh"

// namespace sl::vk {

// class VKFramebuffer : public NonMovable, public NonCopyable {
// public:
//     explicit VKFramebuffer(
//       VKContext& context, VKLogicalDevice& device, VkRenderPass renderPass,
//       u32 width, u32 height, const std::vector<VkImageView>& attachments
//     );

//     VkFramebuffer getHandle();

//     ~VKFramebuffer();

// private:
//     VKContext& m_context;
//     VKLogicalDevice& m_device;

//     std::vector<VkImageView> m_attachments;
//     VkFramebuffer m_handle;
// };

// }  // namespace sl::vk
