#pragma once

#include <vector>
#include <memory>

#include "starlight/renderer/gpu/Swapchain.hh"

#include "Vulkan.hh"
#include "VKTexture.hh"

#include "fwd.hh"

namespace sl::vk {

class VKSwapchain : public Swapchain {
public:
    struct SupportInfo {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    explicit VKSwapchain(
      VkDevice device, Allocator* allocator, VkSurfaceKHR surface,
      u32 graphicsQueueIndex, u32 presentQueueIndex, VkFormat depthFormat,
      u8 depthChannels, const SupportInfo& supportInfo, const Vec2<u32>& size
    );
    ~VKSwapchain();

    void recreate(const Vec2<u32>& size) override;

    std::optional<u32> acquireNextImageIndex(
      Semaphore* imageSemaphore, Fence* fence, Nanoseconds timeout
    ) override;
    u32 getImageCount() const override;

    Texture* getImage(u32 index) override;
    Texture* getDepthBuffer() override;

    VkSurfaceFormatKHR getSurfaceFormat() const;

    VkSwapchainKHR* getHandlePtr();

private:
    void create();
    void destroy();

    void createSwapchain();
    void createImages();

    VkSwapchainKHR m_handle;
    VkDevice m_device;
    Allocator* m_allocator;
    VkSurfaceKHR m_surface;
    u32 m_graphicsQueueIndex;
    u32 m_presentQueueIndex;

    VkFormat m_depthFormat;
    u8 m_depthChannels;

    SupportInfo m_supportInfo;
    Vec2<u32> m_size;

    VkSurfaceFormatKHR m_imageFormat;
    VkExtent2D m_swapchainExtent;

    LocalPtr<VKTexture> m_depthTexture;
    std::vector<LocalPtr<VKSwapchainTexture>> m_textures;

    u32 m_imageCount;
};

VKSwapchain& toVk(Swapchain&);

}  // namespace sl::vk
