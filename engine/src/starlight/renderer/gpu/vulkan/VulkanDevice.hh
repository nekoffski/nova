#pragma once

#include "starlight/core/Core.hh"
#include "starlight/window/Window.hh"
#include "starlight/core/Enum.hh"
#include "starlight/event/EventHandlerSentinel.hh"

#include "starlight/renderer/gpu/Device.hh"
#include "starlight/renderer/gpu/Sync.hh"

#include "fwd.hh"
#include "VulkanQueue.hh"
#include "Vulkan.hh"

namespace sl::vk {

class VulkanDevice : public Device::Impl {
    class Instance : public NonCopyable, public NonMovable {
    public:
        explicit Instance(Allocator* allocator);
        ~Instance();

        VkInstance handle;

    private:
        Allocator* m_allocator;
    };

    class DebugMessenger : public NonCopyable, public NonMovable {
    public:
        explicit DebugMessenger(VkInstance instance, Allocator* allocator);
        ~DebugMessenger();

    private:
        VkInstance m_instance;
        Allocator* m_allocator;
        VkDebugUtilsMessengerEXT m_handle;
    };

    class Surface : public NonCopyable, public NonMovable {
    public:
        explicit Surface(VkInstance instance, Allocator* allocator);
        ~Surface();

        VkSurfaceKHR handle;

    private:
        VkInstance m_instance;
        Allocator* m_allocator;
    };

public:
    using Queues = std::unordered_map<Queue::Type, VulkanQueue>;

    class Physical : public NonCopyable, public NonMovable {
    public:
        using QueueIndices = std::unordered_map<Queue::Type, u32>;

        struct Requirements {
            Queue::Type supportedQueues;
            bool isDiscrete;
            bool supportsSamplerAnisotropy;
            std::vector<const char*> extensions;
        };

        struct Info {
            VkPhysicalDeviceProperties coreProperties;
            VkPhysicalDeviceMemoryProperties memoryProperties;
            VkPhysicalDeviceFeatures features;
            QueueIndices queueIndices;
            VkSurfaceCapabilitiesKHR surfaceCapabilities;
            std::vector<VkSurfaceFormatKHR> surfaceFormats;
            std::vector<VkPresentModeKHR> presentModes;
            VkFormat depthFormat;
            u8 depthChannelCount;
            VkSurfaceFormatKHR surfaceFormat;
            VkPresentModeKHR presentMode;
            bool supportsDeviceLocalHostVisibleMemory;
        };

        explicit Physical(VkInstance instance, VkSurfaceKHR surface);

        VkPhysicalDevice handle;
        Info info;

    private:
    };

    class Logical : public NonCopyable, public NonMovable {
    public:
        Logical(
          VkPhysicalDevice device, Allocator* allocator,
          const Physical::QueueIndices& queueIndices
        );
        ~Logical();

        VkDevice handle;
        VkCommandPool graphicsCommandPool;
        Queues queues;

    private:
        void createDevice(const Physical::QueueIndices& queueIndices);
        void assignQueues(const Physical::QueueIndices& queueIndices);
        void createCommandPool(const Physical::QueueIndices& queueIndices);

        VkPhysicalDevice m_physicalDevice;
        Allocator* m_allocator;
    };

    explicit VulkanDevice();
    ~VulkanDevice() override;

    void waitIdle() override;
    VulkanQueue& getQueue(Queue::Type type) override;

    std::optional<i32> findMemoryIndex(u32 typeFilter, u32 propertyFlags) const;

    EventHandlerSentinel m_eventSentinel;
    Allocator* allocator;
    Instance instance;

private:
    void onWindowResize();
    void createUiResources();

#ifdef SL_VK_DEBUG
    DebugMessenger m_debugMessenger;
#endif

public:
    Surface surface;
    Physical physical;
    Logical logical;
    VkDescriptorPool uiDescriptorPool;
};

}  // namespace sl::vk
