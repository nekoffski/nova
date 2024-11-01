#include "VKLogicalDevice.hh"

namespace sl::vk {

VKLogicalDevice::VKLogicalDevice(
  VKContext& context, VKPhysicalDevice& physicalDevice
) :
    m_context(context), m_physicalDevice(physicalDevice), m_handle(VK_NULL_HANDLE),
    m_graphicsCommandPool(VK_NULL_HANDLE) {
    createLogicalDeviceInstance();
    assignQueues();
    createCommandPool();
    detectDepthFormat();

    LOG_TRACE("VKLogicalDevice created");
}

VKLogicalDevice::~VKLogicalDevice() {
    const auto allocator = m_context.getAllocator();
    if (m_graphicsCommandPool)
        vkDestroyCommandPool(m_handle, m_graphicsCommandPool, allocator);
    if (m_handle) vkDestroyDevice(m_handle, allocator);
}

const VKLogicalDevice::Queues& VKLogicalDevice::getQueues() { return m_queues; }

VkFormat VKLogicalDevice::getDepthFormat() const { return m_depthFormat; }

u8 VKLogicalDevice::getDepthChannelCount() const { return m_depthChannelCount; }

VkCommandPool VKLogicalDevice::getGraphicsCommandPool() {
    return m_graphicsCommandPool;
}

VKPhysicalDevice::SwapchainSupportInfo VKLogicalDevice::getSwapchainSupport() const {
    return m_physicalDevice.getSwapchainSupport();
}

VKPhysicalDevice::DeviceProperties VKLogicalDevice::getDeviceProperties() const {
    return m_physicalDevice.getDeviceProperties();
}

bool VKLogicalDevice::supportsDeviceLocalHostVisibleMemory() const {
    return m_physicalDevice.getDeviceInfo().supportsDeviceLocalHostVisibleMemory;
}

VkDevice VKLogicalDevice::getHandle() { return m_handle; }

std::optional<int32_t> VKLogicalDevice::findMemoryIndex(
  u32 typeFilter, u32 propertyFlags
) const {
    const auto memoryProperties = m_physicalDevice.getDeviceProperties().memory;

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        bool isSuitable =
          (typeFilter & (1 << i))
          && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags
             ) == propertyFlags;
        if (isSuitable) return i;
    }

    LOG_WARN("Unable to find suitable memory type!");
    return {};
}

const VKPhysicalDevice::QueueIndices& VKLogicalDevice::getQueueIndices() const {
    return m_physicalDevice.getDeviceInfo().queueIndices;
}

void VKLogicalDevice::waitIdle() {
    const auto result = vkDeviceWaitIdle(m_handle);
    ASSERT(
      isGood(result), "vkDeviceWaitDile failed: {}", getResultString(result, true)
    );
}

void VKLogicalDevice::createLogicalDeviceInstance() {
    auto queueIndices = m_physicalDevice.getDeviceInfo().queueIndices;

    // prepare queues indices
    static constexpr uint64_t maximumExpectedQueuesCount = 3;
    std::vector<uint32_t> indices;
    indices.reserve(maximumExpectedQueuesCount);
    indices.push_back(queueIndices.graphics);

    if (queueIndices.graphics != queueIndices.present)
        indices.push_back(queueIndices.present);
    if (queueIndices.graphics != queueIndices.transfer)
        indices.push_back(queueIndices.transfer);

    // queue create info
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::vector<float> queueProrities;

    auto queueCount = indices.size();
    queueProrities.reserve(queueCount);
    queueCreateInfos.reserve(queueCount);

    for (const auto index : indices) {
        LOG_TRACE("Adding queue family index: {}", index);

        queueProrities.push_back(1.0f);
        VkDeviceQueueCreateInfo info;
        info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = index;
        info.queueCount       = 1;
        info.flags            = 0;
        info.pNext            = 0;
        info.pQueuePriorities = &queueProrities.back();
        queueCreateInfos.push_back(info);
    }

    // device create info

    VkPhysicalDeviceFeatures deviceFeatures;
    clearMemory(&deviceFeatures);
    deviceFeatures.samplerAnisotropy = VK_TRUE;  // Request anistrophy

    std::vector<const char*> extensionNames = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount    = queueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos       = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures        = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount   = extensionNames.size();
    deviceCreateInfo.ppEnabledExtensionNames = extensionNames.data();

    // Deprecated and ignored, so pass nothing.
    deviceCreateInfo.enabledLayerCount   = 0;
    deviceCreateInfo.ppEnabledLayerNames = 0;

    VK_ASSERT(vkCreateDevice(
      m_physicalDevice.getHandle(), &deviceCreateInfo, m_context.getAllocator(),
      &m_handle
    ));
}

void VKLogicalDevice::assignQueues() {
    const auto queueIndices = m_physicalDevice.getDeviceInfo().queueIndices;

    vkGetDeviceQueue(m_handle, queueIndices.graphics, 0, &m_queues.graphics);
    vkGetDeviceQueue(m_handle, queueIndices.present, 0, &m_queues.present);
    vkGetDeviceQueue(m_handle, queueIndices.transfer, 0, &m_queues.transfer);
}

void VKLogicalDevice::createCommandPool() {
    VkCommandPoolCreateInfo poolCreateInfo;
    clearMemory(&poolCreateInfo);
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    const auto queueIndices = m_physicalDevice.getDeviceInfo().queueIndices;

    poolCreateInfo.queueFamilyIndex = queueIndices.graphics;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_ASSERT(vkCreateCommandPool(
      m_handle, &poolCreateInfo, m_context.getAllocator(), &m_graphicsCommandPool
    ));
}

void VKLogicalDevice::detectDepthFormat() {
    static std::array<std::pair<VkFormat, u8>, 3> candidates = {
        std::make_pair(VK_FORMAT_D32_SFLOAT, 4),
        std::make_pair(VK_FORMAT_D32_SFLOAT_S8_UINT, 4),
        std::make_pair(VK_FORMAT_D24_UNORM_S8_UINT, 3),
    };

    const auto physicalHandle = m_physicalDevice.getHandle();

    uint32_t flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    m_depthFormat  = VK_FORMAT_UNDEFINED;

    for (const auto& [format, channelCount] : candidates) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalHandle, format, &properties);

        if (((properties.linearTilingFeatures & flags) == flags)
            || ((properties.optimalTilingFeatures & flags) == flags)) {
            m_depthFormat       = format;
            m_depthChannelCount = channelCount;
            break;
        }
    }

    ASSERT(
      m_depthFormat != VK_FORMAT_UNDEFINED, "Could not find device depth format"
    );
}

}  // namespace sl::vk
