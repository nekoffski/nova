#pragma once

#include <vector>
#include <unordered_map>
#include <span>

#include "starlight/core/Core.hh"
#include "starlight/core/utils/Enum.hh"

#include "Vulkan.hh"
#include "VKContext.hh"

namespace sl::vk {

class VKPhysicalDevice {
public:
    enum class Queue : uint16_t {
        graphics = 0b0001,
        present  = 0b0010,
        transfer = 0b0100,
        compute  = 0b1000
    };

    struct DeviceProperties {
        static DeviceProperties fetch(VkPhysicalDevice device);

        VkPhysicalDeviceProperties core;
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceMemoryProperties memory;
    };

    struct QueueIndices {
        int32_t graphics = -1;
        int32_t present  = -1;
        int32_t transfer = -1;
        int32_t compute  = -1;
    };

    struct SwapchainSupportInfo {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct Requirements {
        Queue supportedQueues;
        bool isDiscreteGPU;
        bool supportsSamplerAnisotropy;
        std::span<const char*> extensions;
    };

    struct DeviceInfo {
        QueueIndices queueIndices;
        bool supportsDeviceLocalHostVisibleMemory;
    };

public:
    explicit VKPhysicalDevice(VKContext& context, const Requirements& requirements);

    VkPhysicalDevice getHandle();

    const DeviceInfo& getDeviceInfo() const;
    const DeviceProperties& getDeviceProperties() const;
    SwapchainSupportInfo getSwapchainSupport();

private:
    std::vector<VkPhysicalDevice> getPhysicalDevices() const;

    void logQueueIndices(const QueueIndices&) const;
    bool validateQueueIndices(const QueueIndices&) const;
    bool validateExtensions(VkPhysicalDevice) const;

    SwapchainSupportInfo queryDeviceSwapchainSupport(VkPhysicalDevice) const;

    std::optional<DeviceInfo> isPhysicalDeviceSuitable(
      VkPhysicalDevice device, const DeviceProperties& properties
    ) const;

    std::optional<VKPhysicalDevice::QueueIndices> findQueueIndices(
      VkPhysicalDevice device
    ) const;

    void storeDevice(
      VkPhysicalDevice device, const DeviceProperties& properties,
      const DeviceInfo& info
    );

    VKContext& m_context;
    Requirements m_requirements;
    VkPhysicalDevice m_handle;
    DeviceInfo m_deviceInfo;
    DeviceProperties m_deviceProperties;
};

constexpr void enableBitOperations(VKPhysicalDevice::Queue);

}  // namespace sl::vk
