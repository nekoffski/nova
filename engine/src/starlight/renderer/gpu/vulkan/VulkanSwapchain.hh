#pragma once

#include <vector>
#include <memory>

#include "starlight/renderer/gpu/Swapchain.hh"

#include "Vulkan.hh"
#include "VulkanTexture.hh"

#include "fwd.hh"

namespace sl::vk {

class VulkanSwapchain : public Swapchain {
public:
    explicit VulkanSwapchain(VulkanDevice& device, const Vec2<u32>& size);
    ~VulkanSwapchain() override;

    void recreate(const Vec2<u32>& size) override;

    std::optional<u32> acquireNextImageIndex(
      Semaphore* imageSemaphore, Fence* fence, Nanoseconds timeout
    ) override;
    u32 getImageCount() const override;

    Texture* getImage(u32 index) override;
    Texture* getDepthBuffer() override;

    VkSwapchainKHR* getHandlePtr();

private:
    void create();
    void destroy();

    void createSwapchain();
    void createImages();

    VkSwapchainKHR m_handle;
    VulkanDevice& m_device;

    Vec2<u32> m_size;

    VkExtent2D m_swapchainExtent;

    LocalPointer<VulkanTexture> m_depthTexture;
    std::vector<LocalPointer<VulkanSwapchainTexture>> m_textures;

    u32 m_imageCount;
};

VulkanSwapchain& toVk(Swapchain&);

}  // namespace sl::vk
