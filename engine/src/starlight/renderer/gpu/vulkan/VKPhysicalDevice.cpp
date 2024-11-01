#include "VKPhysicalDevice.hh"

#include <kc/core/Utils.hpp>

#include <optional>

namespace sl::vk {

VKPhysicalDevice::DeviceProperties VKPhysicalDevice::DeviceProperties::fetch(
  VkPhysicalDevice device
) {
    DeviceProperties properties;

    vkGetPhysicalDeviceProperties(device, &properties.core);
    vkGetPhysicalDeviceFeatures(device, &properties.features);
    vkGetPhysicalDeviceMemoryProperties(device, &properties.memory);

    return properties;
}

// TODO: physical device picker or something
VKPhysicalDevice::VKPhysicalDevice(
  VKContext& context, const Requirements& requirements
) : m_context(context), m_requirements(requirements), m_handle(VK_NULL_HANDLE) {
    for (const auto& physicalDevice : getPhysicalDevices()) {
        auto properties = DeviceProperties::fetch(physicalDevice);

        LOG_INFO("Checking device: '{}'.", properties.core.deviceName);
        if (const auto deviceInfo =
              isPhysicalDeviceSuitable(physicalDevice, properties);
            deviceInfo) {
            storeDevice(physicalDevice, properties, *deviceInfo);
            break;
        }
    }
    ASSERT(
      m_handle,
      "Could not pick suitable device for vulkan, no GPU or it doesn't provide all necessary features"
    );
    LOG_INFO("Physical device selected");
}

VkPhysicalDevice VKPhysicalDevice::getHandle() { return m_handle; }

const VKPhysicalDevice::DeviceInfo& VKPhysicalDevice::getDeviceInfo() const {
    return m_deviceInfo;
}

const VKPhysicalDevice::DeviceProperties& VKPhysicalDevice::getDeviceProperties(
) const {
    return m_deviceProperties;
}

VKPhysicalDevice::SwapchainSupportInfo VKPhysicalDevice::getSwapchainSupport() {
    return queryDeviceSwapchainSupport(m_handle);
}

std::vector<VkPhysicalDevice> VKPhysicalDevice::getPhysicalDevices() const {
    const auto instance = m_context.getInstance();

    uint32_t deviceCount = 0;
    VK_ASSERT(vkEnumeratePhysicalDevices(instance, &deviceCount, 0));

    ASSERT(deviceCount > 0, "Could not find any Vulkan Physical VKDevice");

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    VK_ASSERT(
      vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data())
    );

    return physicalDevices;
}

void VKPhysicalDevice::logQueueIndices(const QueueIndices& indices) const {
    LOG_INFO("VKDevice meets queue requirements.");
    LOG_INFO("Graphics Family Index: {}", indices.graphics);
    LOG_INFO("Present  Family Index: {}", indices.present);
    LOG_INFO("Transfer Family Index: {}", indices.transfer);
    LOG_INFO("Compute  Family Index: {}", indices.compute);
}

bool VKPhysicalDevice::validateQueueIndices(const QueueIndices& indices) const {
    if (isFlagEnabled(m_requirements.supportedQueues, Queue::graphics)
        && indices.graphics == -1) {
        LOG_WARN("Device doesn't support graphics queue");
        return false;
    }
    if (isFlagEnabled(m_requirements.supportedQueues, Queue::compute)
        && indices.compute == -1) {
        LOG_WARN("Device doesn't support compute queue");
        return false;
    }
    if (isFlagEnabled(m_requirements.supportedQueues, Queue::transfer)
        && indices.transfer == -1) {
        LOG_WARN("Device doesn't support transfer queue");
        return false;
    }
    if (isFlagEnabled(m_requirements.supportedQueues, Queue::present)
        && indices.present == -1) {
        LOG_WARN("Device doesn't support present queue");
        return false;
    }
    return true;
}

bool VKPhysicalDevice::validateExtensions(VkPhysicalDevice device) const {
    uint32_t availableExtensionCount = 0;
    std::vector<VkExtensionProperties> availableExtenions;

    VK_ASSERT(
      vkEnumerateDeviceExtensionProperties(device, 0, &availableExtensionCount, 0)
    );
    availableExtenions.resize(availableExtensionCount);

    VK_ASSERT(vkEnumerateDeviceExtensionProperties(
      device, 0, &availableExtensionCount, availableExtenions.data()
    ));

    std::vector<const char*> availableExtensionsNames;
    availableExtenions.reserve(availableExtensionCount);

    std::ranges::transform(
      availableExtenions, std::back_inserter(availableExtensionsNames),
      [](const auto& extension) -> const char* { return extension.extensionName; }
    );

    for (const auto& requiredExtension : m_requirements.extensions) {
        if (not kc::core::contains(availableExtensionsNames, requiredExtension)) {
            LOG_INFO("Extension {} not available", requiredExtension);
            return false;
        }
    }

    return true;
}

VKPhysicalDevice::SwapchainSupportInfo VKPhysicalDevice::queryDeviceSwapchainSupport(
  VkPhysicalDevice physicalDevice
) const {
    const auto surface = m_context.getSurface();
    SwapchainSupportInfo result;

    // Surface capabilities
    VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physicalDevice, surface, &result.capabilities
    ));

    uint32_t count = 0;
    VK_ASSERT(
      vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, 0)
    );

    if (count != 0) {
        result.formats.resize(count);
        VK_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(
          physicalDevice, surface, &count, result.formats.data()
        ));
    }

    count = 0;
    VK_ASSERT(
      vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, 0)
    );

    if (count != 0) {
        result.presentModes.resize(count);
        VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(
          physicalDevice, surface, &count, result.presentModes.data()
        ));
    }

    return result;
}

std::optional<VKPhysicalDevice::QueueIndices> VKPhysicalDevice::findQueueIndices(
  VkPhysicalDevice device
) const {
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
      device, &queueFamilyCount, queueFamilies.data()
    );

    QueueIndices indices;
    uint8_t minTransferScore = 255;

    for (u32 i = 0; i < queueFamilyCount; ++i) {
        const auto& queueFlags       = queueFamilies[i].queueFlags;
        uint8_t currentTransferScore = 0;

        if (queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics = i;
            ++currentTransferScore;
        }

        if (queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.compute = i;
            ++currentTransferScore;
        }

        if (queueFlags & VK_QUEUE_TRANSFER_BIT) {
            // Take the index if it is the current lowest.This increases the
            // liklihood that it is a dedicated transfer queue.
            if (currentTransferScore <= minTransferScore) {
                minTransferScore = currentTransferScore;
                indices.transfer = i;
            }
        }

        VkBool32 supportsPresent = false;
        VK_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(
          device, i, m_context.getSurface(), &supportsPresent
        ));

        if (supportsPresent) indices.present = i;
    }

    if (validateQueueIndices(indices)) return indices;
    return {};
}

std::optional<VKPhysicalDevice::DeviceInfo>
  VKPhysicalDevice::isPhysicalDeviceSuitable(
    VkPhysicalDevice device, const DeviceProperties& properties
  ) const {
    DeviceInfo result;

    if (m_requirements.isDiscreteGPU
        && properties.core.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        LOG_INFO(
          "VKDevice is not a discrete GPU, and one is required, skipping device"
        );
        return {};
    }

    const auto queueIndices = findQueueIndices(device);

    if (not queueIndices) {
        LOG_INFO("Could not satisfy queues requirements, skipping device");
        return {};
    }
    logQueueIndices(*queueIndices);
    result.queueIndices = *queueIndices;

    if (const auto swapchainSupport = queryDeviceSwapchainSupport(device);
        swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty()) {
        LOG_INFO("Required swapchain support not present, skipping device.");
        return {};
    }

    if (not m_requirements.extensions.empty() && not validateExtensions(device)) {
        LOG_WARN("Device doesn't provide required extensions, skipping");
        return {};
    }

    if (m_requirements.supportsSamplerAnisotropy
        && !properties.features.samplerAnisotropy) {
        LOG_INFO("VKDevice does not support samplerAnisotropy, skipping.");
        return {};
    }

    // Check if device supports local/host visible combo
    for (u32 i = 0; i < properties.memory.memoryTypeCount; ++i) {
        if ((properties.memory.memoryTypes[i].propertyFlags
             & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            && (properties.memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
            result.supportsDeviceLocalHostVisibleMemory = true;
            break;
        }
    }

    return result;
}

static void showDeviceType(const VkPhysicalDeviceProperties& properties) {
    switch (properties.deviceType) {
        default:
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            LOG_INFO("GPU type is Unknown");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            LOG_INFO("GPU type is Integrated");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            LOG_INFO("GPU type is Discrete");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            LOG_INFO("GPU type is Virtual");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            LOG_INFO("GPU type is CPU");
            break;
    }
}

static void showDeviceInfo(
  const VkPhysicalDeviceProperties& properties,
  const VkPhysicalDeviceMemoryProperties& memory
) {
    LOG_INFO("Selected device: '{}'.", properties.deviceName);
    showDeviceType(properties);

    LOG_INFO(
      "GPU Driver version: {}.{}.{}", VK_VERSION_MAJOR(properties.driverVersion),
      VK_VERSION_MINOR(properties.driverVersion),
      VK_VERSION_PATCH(properties.driverVersion)
    );

    LOG_INFO(
      "Vulkan API version: {}.{}.{}", VK_VERSION_MAJOR(properties.apiVersion),
      VK_VERSION_MINOR(properties.apiVersion),
      VK_VERSION_PATCH(properties.apiVersion)
    );

    // Memory information
    for (uint32_t j = 0; j < memory.memoryHeapCount; ++j) {
        const float memorySize =
          (((float)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
        if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            LOG_INFO("Local GPU memory: {} GiB", memorySize);
        else
            LOG_INFO("Shared System memory: {} GiB", memorySize);
    }
}

void VKPhysicalDevice::storeDevice(
  VkPhysicalDevice device, const DeviceProperties& properties, const DeviceInfo& info
) {
    showDeviceInfo(properties.core, properties.memory);

    m_handle           = device;
    m_deviceInfo       = info;
    m_deviceProperties = properties;
}

}  // namespace sl::vk
