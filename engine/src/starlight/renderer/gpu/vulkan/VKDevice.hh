#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/window/Window.hh"
#include "starlight/core/Config.hh"
#include "starlight/core/utils/Enum.hh"

#include "starlight/renderer/gpu/Device.hh"
#include "starlight/renderer/gpu/Sync.hh"

#include "VKQueue.hh"
#include "Vulkan.hh"

namespace sl::vk {

class VKDevice : public Device {
    class VKInstance : public NonCopyable, public NonMovable {
    public:
        explicit VKInstance(const Config& config, Allocator* allocator);
        ~VKInstance();

        VkInstance handle;

    private:
        Allocator* m_allocator;
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

        VkSurfaceKHR handle;

    private:
        VkInstance m_instance;
        Allocator* m_allocator;
    };

public:
    using Queues = std::unordered_map<Queue::Type, VKQueue>;

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
        Queues queues;

    private:
        void createDevice(const Physical::QueueIndices& queueIndices);
        void assignQueues(const Physical::QueueIndices& queueIndices);
        void createCommandPool(const Physical::QueueIndices& queueIndices);

        VkPhysicalDevice m_physicalDevice;
        Allocator* m_allocator;
        VkCommandPool m_graphicsCommandPool;
    };

    explicit VKDevice(Window& window, const Config& config);
    ~VKDevice();

    void waitIdle() override;
    Queue& getQueue(Queue::Type type) override;

    Allocator* getAllocator();
    VkInstance getInstance();
    VkSurfaceKHR getSurface();

    OwningPtr<Semaphore> createSemaphore() override;
    OwningPtr<Fence> createFence(Fence::State) override;
    OwningPtr<Swapchain> createSwapchain(const Vec2<u32>& size) override;

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

}  // namespace sl::vk
