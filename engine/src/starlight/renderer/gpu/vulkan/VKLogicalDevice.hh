// #pragma once

// #include "VKContext.hh"
// #include "VKPhysicalDevice.hh"

// namespace sl::vk {

// class VKLogicalDevice : public NonCopyable, public NonMovable {
// public:
//     struct Queues {
//         VkQueue graphics;
//         VkQueue present;
//         VkQueue transfer;
//     };

//     explicit VKLogicalDevice(VKContext& context, VKPhysicalDevice&
//     physicalDevice); ~VKLogicalDevice();

//     const Queues& getQueues();
//     VkFormat getDepthFormat() const;
//     u8 getDepthChannelCount() const;
//     VkCommandPool getGraphicsCommandPool();

//     VKPhysicalDevice::SwapchainSupportInfo getSwapchainSupport() const;
//     VKPhysicalDevice::DeviceProperties getDeviceProperties() const;
//     bool supportsDeviceLocalHostVisibleMemory() const;

//     std::optional<int32_t> findMemoryIndex(u32 typeFilter, u32 propertyFlags)
//     const;

//     const VKPhysicalDevice::QueueIndices& getQueueIndices() const;

//     VkDevice getHandle();

//     void waitIdle();

// private:
//     void createLogicalDeviceInstance();
//     void assignQueues();
//     void createCommandPool();
//     void detectDepthFormat();

//     VKContext& m_context;
//     VKPhysicalDevice& m_physicalDevice;

//     VkDevice m_handle;
//     VkCommandPool m_graphicsCommandPool;

//     Queues m_queues;
//     VkFormat m_depthFormat;
//     u8 m_depthChannelCount;
// };

// }  // namespace sl::vk
