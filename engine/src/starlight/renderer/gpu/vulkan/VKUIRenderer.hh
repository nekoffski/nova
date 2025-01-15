// #pragma once

// #include "starlight/core/window/Window.hh"

// #include "starlight/renderer/gpu/UIRenderer.hh"

// #include "VKPhysicalDevice.hh"
// #include "VKContext.hh"

// #include "VulkanCommandBuffer.hh"
// #include "Vulkan.hh"
// #include "fwd.hh"

// namespace sl::vk {

// class VKUIRenderer : public UIRenderer {
// public:
//     explicit VKUIRenderer(
//       VKContext& context, VKPhysicalDevice& physicalDevice,
//       VKLogicalDevice& logicalDevice, Window& window, RenderPass& renderPass
//     );

//     ~VKUIRenderer() override;

//     void reloadFontTextures() override;

// private:
//     void begin(CommandBuffer& commandBuffer) override;
//     void end(CommandBuffer& commandBuffer) override;

//     VKContext& m_context;
//     VKPhysicalDevice& m_physicalDevice;
//     VKLogicalDevice& m_device;

//     VkDescriptorPool m_uiPool;

//     // allow only single instance per engine
//     inline static bool s_hasInstance = false;
// };

// }  // namespace sl::vk
