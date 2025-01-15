// #pragma once

// #include "starlight/core/math/Core.hh"

// #include "starlight/renderer/gpu/RenderPass.hh"

// #include "VKPhysicalDevice.hh"
// #include "VKContext.hh"

// #include "Vulkan.hh"
// #include "fwd.hh"

// namespace sl::vk {

// class VKRenderPass : public RenderPass {
// public:
//     explicit VKRenderPass(
//       VKContext& context, VKLogicalDevice& device, const VulkanSwapchain&
//       swapchain, const Properties& properties, ChainFlags chainFlags
//     );

//     ~VKRenderPass();

//     void begin(CommandBuffer& commandBuffer, u8 attachmentIndex) override;
//     void end(CommandBuffer& commandBuffer) override;

//     void regenerateRenderTargets(const Vec2<u32>& viewportSize) override;

//     VkRenderPass getHandle();

// private:
//     void generateRenderTargets();

//     std::vector<VkClearValue> createClearValues(ClearFlags flags) const;

//     VkRenderPassBeginInfo createRenderPassBeginInfo(
//       const std::vector<VkClearValue>& clearValues, VkFramebuffer framebuffer
//     ) const;

//     VKContext& m_context;
//     VKLogicalDevice& m_device;

//     VkRenderPass m_handle;
//     float m_depth      = 1.0f;
//     uint32_t m_stencil = 0;

//     State m_state;

//     bool m_hasColorAttachment;
//     bool m_hasDepthAttachment;

//     // TODO: use local mem pool or static array or something
//     std::vector<OwningPtr<VKFramebuffer>> m_framebuffers;
// };

// }  // namespace sl::vk
