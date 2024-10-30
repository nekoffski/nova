#include "VKSwapchain.hh"

#include <kc/core/Log.h>
#include <kc/core/Utils.hpp>

#include "VKImage.hh"
#include "VKFramebuffer.hh"

namespace sl::vk {

VKSwapchain::VKSwapchain(
  VKContext& context, VKLogicalDevice& device, u32 viewportWidth, u32 viewportHeight
) :
    m_context(context), m_device(device), m_viewportWidth(viewportWidth),
    m_viewportHeight(viewportHeight) {
    create();
}

VkSurfaceFormatKHR pickSurfaceFormat(
  const VKPhysicalDevice::SwapchainSupportInfo& swapchainSupport
) {
    const auto demandedFormat     = VK_FORMAT_R8G8B8A8_UNORM;
    const auto demandedColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    for (const auto& format : swapchainSupport.formats)
        if (format.format == demandedFormat
            && format.colorSpace == demandedColorSpace)
            return format;

    return swapchainSupport.formats[0];
}

VkPresentModeKHR pickPresentMode(
  const VKPhysicalDevice::SwapchainSupportInfo& swapchainSupport
) {
    const auto defaultPresentMode  = VK_PRESENT_MODE_FIFO_KHR;
    const auto demandedPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

    if (kc::core::contains(swapchainSupport.presentModes, demandedPresentMode))
        return demandedPresentMode;

    return defaultPresentMode;
}

VkExtent2D createSwapchainExtent(
  u32 viewportWidth, u32 viewportHeight,
  const VKPhysicalDevice::SwapchainSupportInfo& swapchainSupport
) {
    VkExtent2D swapchainExtent = { viewportWidth, viewportHeight };

    // if (swapchainSupport.capabilities.currentExtent.width != UINT32_MAX) {
    // LOG_INFO("mamy cie", viewportWidth, viewportHeight);

    // swapchainExtent = swapchainSupport.capabilities.currentExtent;
    // }

    // Clamp to the value allowed by the GPU.bonestent.width, min.width, max.width);
    auto& min = swapchainSupport.capabilities.minImageExtent;
    auto& max = swapchainSupport.capabilities.minImageExtent;

    swapchainExtent.height =
      std::clamp(swapchainExtent.height, min.height, max.height);

    return swapchainExtent;
}

VKTexture* VKSwapchain::getFramebuffer(u64 id) {
    ASSERT(id < m_textures.size(), "Invalid framebuffer id - {}", id);
    return m_textures[id].get();
}

VKTexture* VKSwapchain::getDepthBuffer() { return m_depthTexture.get(); }

uint32_t getDeviceImageCount(
  const VKPhysicalDevice::SwapchainSupportInfo& swapchainSupport
) {
    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;

    if (swapchainSupport.capabilities.maxImageCount > 0
        && imageCount > swapchainSupport.capabilities.maxImageCount) {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    return imageCount;
}

VkSurfaceFormatKHR VKSwapchain::getSurfaceFormat() const { return m_imageFormat; }

std::span<LocalPtr<VKTexture>> VKSwapchain::getTextures() { return m_textures; }

struct SwapchainCreateInfo {
    VkSwapchainCreateInfoKHR handle;
    std::vector<uint32_t> queueFamilyIndices;
};

SwapchainCreateInfo createSwapchainCreateInfo(
  VKLogicalDevice& device, VkSurfaceKHR surface, uint32_t imageCount,
  VkSurfaceFormatKHR imageFormat, VkExtent2D extent, VkPresentModeKHR presentMode
) {
    SwapchainCreateInfo createInfo;
    auto& handle = createInfo.handle;

    handle.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    handle.surface          = surface;
    handle.minImageCount    = imageCount;
    handle.imageFormat      = imageFormat.format;
    handle.imageColorSpace  = imageFormat.colorSpace;
    handle.imageExtent      = extent;
    handle.imageArrayLayers = 1;
    handle.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    handle.pNext            = nullptr;
    handle.flags            = 0;

    const auto& queueIndices = device.getQueueIndices();

    createInfo.queueFamilyIndices = {
        static_cast<uint32_t>(queueIndices.graphics),
        static_cast<uint32_t>(queueIndices.present),
    };

    // Setup the queue family indices
    if (queueIndices.graphics != queueIndices.present) {
        handle.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        handle.queueFamilyIndexCount = 2;
        handle.pQueueFamilyIndices   = createInfo.queueFamilyIndices.data();
    } else {
        handle.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        handle.queueFamilyIndexCount = 0;
        handle.pQueueFamilyIndices   = 0;
    }

    handle.preTransform = device.getSwapchainSupport().capabilities.currentTransform;
    handle.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    handle.presentMode    = presentMode;
    handle.clipped        = VK_TRUE;
    handle.oldSwapchain   = 0;

    return createInfo;
}

VkImageViewCreateInfo createImageViewCreateInfo(VkImage image, VkFormat format) {
    VkImageViewCreateInfo viewInfo;
    viewInfo.sType                       = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image                       = image;
    viewInfo.viewType                    = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format                      = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    return viewInfo;
}

void VKSwapchain::createSwapchain() {
    auto swapchainSupport = m_device.getSwapchainSupport();

    m_imageFormat    = pickSurfaceFormat(swapchainSupport);
    auto presentMode = pickPresentMode(swapchainSupport);

    m_swapchainExtent =
      createSwapchainExtent(m_viewportWidth, m_viewportHeight, swapchainSupport);
    auto deviceImageCount = getDeviceImageCount(swapchainSupport);

    LOG_INFO(
      "Creating swapchain: {}/{}", m_swapchainExtent.width, m_swapchainExtent.height
    );

    auto swapchainCreateInfo = createSwapchainCreateInfo(
      m_device, m_context.getSurface(), deviceImageCount, m_imageFormat,
      m_swapchainExtent, presentMode
    );

    VK_ASSERT(vkCreateSwapchainKHR(
      m_device.getHandle(), &swapchainCreateInfo.handle, m_context.getAllocator(),
      &m_handle
    ));
}

void VKSwapchain::createImages() {
    const auto logicalDeviceHandle = m_device.getHandle();

    VK_ASSERT(
      vkGetSwapchainImagesKHR(logicalDeviceHandle, m_handle, &m_imageCount, 0)
    );

    ASSERT(m_imageCount > 0, "swapchainImageCount==0 for vulkan swapchain");

    std::array<VkImage, 16> swapchainImages = { 0 };

    VK_ASSERT(vkGetSwapchainImagesKHR(
      logicalDeviceHandle, m_handle, &m_imageCount, swapchainImages.data()
    ));

    if (m_textures.size() != m_imageCount) {
        m_textures.clear();
        m_textures.resize(m_imageCount);

        VKTexture::Properties props{};
        props.width         = m_swapchainExtent.width;
        props.height        = m_swapchainExtent.height;
        props.channels      = 4;
        props.isTransparent = false;
        props.isWritable    = true;

        for (u32 i = 0; i < m_imageCount; ++i) {
            auto& swapchainImageHandle = swapchainImages[i];
            props.name = fmt::format("SL_InternalSwapchainTexture_{}", i);

            m_textures[i].emplace(
              m_context, m_device, props, swapchainImageHandle, m_imageFormat.format
            );
        }
    } else {
        for (u32 i = 0; i < m_imageCount; ++i) {
            auto& texture = m_textures[i];
            texture->resize(m_swapchainExtent.width, m_swapchainExtent.height);
        }
    }

    // Create depth image and its view.
    VKImage::Properties imageProperties{
        VKImage::Type::flat,
        m_swapchainExtent.width,
        m_swapchainExtent.height,
        m_device.getDepthFormat(),
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        m_device.getDepthChannelCount()
    };

    m_depthTexture.emplace(m_context, m_device, imageProperties);
}

void VKSwapchain::create() {
    createSwapchain();
    createImages();
    LOG_INFO("VKSwapchain created successfully.");
}

VKSwapchain::~VKSwapchain() { destroy(); }

void VKSwapchain::destroy() {
    vkDestroySwapchainKHR(m_device.getHandle(), m_handle, m_context.getAllocator());
    m_textures.clear();
    m_depthTexture.clear();
    LOG_TRACE("VKSwapchain destroyed");
}

std::optional<uint32_t> VKSwapchain::acquireNextImageIndex(
  Nanoseconds timeout, VkSemaphore imageSemaphore, VkFence fence
) {
    uint32_t index;

    VkResult result = vkAcquireNextImageKHR(
      m_device.getHandle(), m_handle, timeout, imageSemaphore, fence, &index
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // Trigger swapchain recreation, then boot out of the render loop.
        // destroy();
        // create();
        return {};
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOG_ERROR("Failed to acquire swapchain image!");
        return {};
    }

    return index;
}

VkPresentInfoKHR createPresentInfo(
  VkSemaphore* renderSemaphore, VkSwapchainKHR* swapchain,
  uint32_t* presentImageIndex
) {
    // Return the image to the swapchain for presentation.
    VkPresentInfoKHR presentInfo;
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = renderSemaphore;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = swapchain;
    presentInfo.pImageIndices      = presentImageIndex;
    presentInfo.pResults           = 0;

    return presentInfo;
}

void VKSwapchain::present(
  [[maybe_unused]] VkQueue graphicsQueue, VkQueue presentQueue,
  VkSemaphore renderSemaphore, uint32_t presentImageIndex
) {
    const auto presentInfo =
      createPresentInfo(&renderSemaphore, &m_handle, &presentImageIndex);

    if (VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
        result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // VKSwapchain is out of date, suboptimal or a framebuffer resize has
        // occurred. Trigger swapchain recreation
        // destroy();
        // create();
    } else if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to present swap chain image!");
    }
}

void VKSwapchain::recreate(u32 viewportWidth, u32 viewportHeight) {
    LOG_INFO("Recreating swapchain: {}/{}", viewportWidth, viewportHeight);

    m_viewportWidth  = viewportWidth;
    m_viewportHeight = viewportHeight;

    destroy();
    create();
}

u32 VKSwapchain::getImageCount() const { return m_imageCount; }

}  // namespace sl::vk
