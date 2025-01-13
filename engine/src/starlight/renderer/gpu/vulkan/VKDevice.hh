#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/window/Window.hh"
#include "starlight/core/Config.hh"
#include "starlight/core/utils/Enum.hh"
#include "starlight/renderer/gpu/Device.hh"
#include "Vulkan.hh"

namespace sl::vk {

class VKDevice : public Device {
    class VKInstance : public NonCopyable, public NonMovable {
    public:
        explicit VKInstance(const Config& config, Allocator* allocator);
        ~VKInstance();

        VkInstance get();

    private:
        Allocator* m_allocator;
        VkInstance m_handle;
    };

    class VKDebugMessenger : public NonCopyable, public NonMovable {
    public:
        explicit VKDebugMessenger(VkInstance instance, Allocator* allocator);
        ~VKDebugMessenger();

    private:
        VkInstance m_instance;
        Allocator* m_allocator;
        VkDebugUtilsMessengerEXT m_handle;
    };

    class VKSurface : public NonCopyable, public NonMovable {
    public:
        explicit VKSurface(
          VkInstance instance, Window& window, Allocator* allocator
        );
        ~VKSurface();

        VkSurfaceKHR get();

    private:
        VkInstance m_instance;
        Allocator* m_allocator;
        VkSurfaceKHR m_handle;
    };

public:
    enum class QueueType : u8 {
        none     = 0x0,
        graphics = 0x1,
        present  = 0x2,
        transfer = 0x4,
        compute  = 0x8
    };

    class Physical : public NonCopyable, public NonMovable {
    public:
        struct Requirements {
            QueueType supportedQueues;
            bool isDiscrete;
            bool supportsSamplerAnisotropy;
            std::vector<const char*> extensions;
        };

        struct Info {
            VkPhysicalDeviceProperties coreProperties;
            VkPhysicalDeviceMemoryProperties memoryProperties;
            VkPhysicalDeviceFeatures features;
            std::unordered_map<QueueType, u32> queueIndices;
            VkSurfaceCapabilitiesKHR surfaceCapabilities;
            std::vector<VkSurfaceFormatKHR> surfaceFormats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        explicit Physical(VkInstance instance, VkSurfaceKHR surface);

        VkPhysicalDevice get();

    private:
        Info m_info;
        VkPhysicalDevice m_handle;
    };

    class Logical : public NonCopyable, public NonMovable {
    public:
        Logical(VkPhysicalDevice device, Allocator* allocator);
        ~Logical();

        VkDevice get();

    private:
        VkDevice m_handle;
        Allocator* m_allocator;
        VkCommandPool m_graphicsCommandPool;
    };

    explicit VKDevice(Window& window, const Config& config);
    ~VKDevice();

    Allocator* getAllocator();
    VkInstance getInstance();
    VkSurfaceKHR getSurface();

private:
    Allocator* m_allocator;
    VKInstance m_instance;

#ifdef SL_VK_DEBUG
    VKDebugMessenger m_debugMessenger;
#endif

    VKSurface m_surface;
    Physical m_physicalDevice;
    Logical m_logicalDevice;
};

constexpr void enableBitOperations(VKDevice::QueueType);

}  // namespace sl::vk
