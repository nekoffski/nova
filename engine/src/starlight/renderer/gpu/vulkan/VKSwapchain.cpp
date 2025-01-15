#include "VKSwapchain.hh"

#include <kc/core/Log.h>
#include <kc/core/Utils.hpp>

#include "VKFramebuffer.hh"
#include "VKSemaphore.hh"
#include "VKDevice.hh"
#include "VKFence.hh"

namespace sl::vk {

VKSwapchain::VKSwapchain(
  VkDevice device, Allocator* allocator, VkSurfaceKHR surface,
  u32 graphicsQueueIndex, u32 presentQueueIndex, VkFormat depthFormat,
  u8 depthChannels, const SupportInfo& supportInfo, const Vec2<u32>& size
) :
    m_handle(VK_NULL_HANDLE), m_device(device), m_allocator(allocator),
    m_surface(surface), m_graphicsQueueIndex(graphicsQueueIndex),
    m_presentQueueIndex(presentQueueIndex), m_depthFormat(depthFormat),
    m_depthChannels(depthChannels), m_supportInfo(supportInfo), m_size(size) {
    create();
}

VkSurfaceFormatKHR pickSurfaceFormat(const VKSwapchain::SupportInfo& swapchainSupport
) {
    static const auto demandedFormat     = VK_FORMAT_R8G8B8A8_UNORM;
    static const auto demandedColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    for (const auto& format : swapchainSupport.surfaceFormats)
        if (format.format == demandedFormat
            && format.colorSpace == demandedColorSpace)
            return format;
    return swapchainSupport.surfaceFormats[0];
}

VkPresentModeKHR pickPresentMode(const VKSwapchain::SupportInfo& swapchainSupport) {
    static const auto defaultPresentMode  = VK_PRESENT_MODE_FIFO_KHR;
    static const auto demandedPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

    if (kc::core::contains(swapchainSupport.presentModes, demandedPresentMode))
        return demandedPresentMode;
    return defaultPresentMode;
}

VkExtent2D createSwapchainExtent(
  u32 viewportWidth, u32 viewportHeight,
  const VKSwapchain::SupportInfo& swapchainSupport
) {
    VkExtent2D swapchainExtent = { viewportWidth, viewportHeight };

    const auto min = swapchainSupport.surfaceCapabilities.minImageExtent;
    const auto max = swapchainSupport.surfaceCapabilities.maxImageExtent;

    LOG_INFO(
      "Swapchain capabilities min/max - {} - {}/{} - {}", min.width, min.height,
      max.width, max.height
    );

    swapchainExtent.height =
      std::clamp(swapchainExtent.height, min.height, max.height);

    return swapchainExtent;
}

Texture* VKSwapchain::getImage(u32 id) {
    ASSERT(id < m_textures.size(), "Invalid image id - {}", id);
    return m_textures[id].get();
}

Texture* VKSwapchain::getDepthBuffer() { return m_depthTexture.get(); }

static u8 getDeviceImageCount(const VKSwapchain::SupportInfo& swapchainSupport) {
    auto imageCount = swapchainSupport.surfaceCapabilities.minImageCount + 1;

    if (swapchainSupport.surfaceCapabilities.maxImageCount > 0
        && imageCount > swapchainSupport.surfaceCapabilities.maxImageCount) {
        imageCount = swapchainSupport.surfaceCapabilities.maxImageCount;
    }

    return imageCount;
}

VkSurfaceFormatKHR VKSwapchain::getSurfaceFormat() const { return m_imageFormat; }

VkSwapchainKHR* VKSwapchain::getHandlePtr() { return &m_handle; }

struct SwapchainCreateInfo {
    explicit SwapchainCreateInfo() { clearMemory(&handle); }

    VkSwapchainCreateInfoKHR handle;
    std::vector<u32> queueFamilyIndices;
};

SwapchainCreateInfo createSwapchainCreateInfo(
  VkDevice device, VkSurfaceKHR surface, u32 imageCount, u32 graphicsQueue,
  u32 presentQueue, VkSurfaceFormatKHR imageFormat, VkExtent2D extent,
  VkPresentModeKHR presentMode, const VKSwapchain::SupportInfo& swapchainSupport
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

    createInfo.queueFamilyIndices = { graphicsQueue, presentQueue };

    // Setup the queue family indices
    if (graphicsQueue != presentQueue) {
        handle.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        handle.queueFamilyIndexCount = 2;
        handle.pQueueFamilyIndices   = createInfo.queueFamilyIndices.data();
    } else {
        handle.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        handle.queueFamilyIndexCount = 0;
        handle.pQueueFamilyIndices   = 0;
    }

    handle.preTransform   = swapchainSupport.surfaceCapabilities.currentTransform;
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
    m_imageFormat    = pickSurfaceFormat(m_supportInfo);
    auto presentMode = pickPresentMode(m_supportInfo);

    m_swapchainExtent     = createSwapchainExtent(m_size.w, m_size.h, m_supportInfo);
    auto deviceImageCount = getDeviceImageCount(m_supportInfo);

    LOG_INFO(
      "Creating swapchain: {}/{}", m_swapchainExtent.width, m_swapchainExtent.height
    );

    auto swapchainCreateInfo = createSwapchainCreateInfo(
      m_device, m_surface, deviceImageCount, m_graphicsQueueIndex,
      m_presentQueueIndex, m_imageFormat, m_swapchainExtent, presentMode,
      m_supportInfo
    );

    VK_ASSERT(vkCreateSwapchainKHR(
      m_device, &swapchainCreateInfo.handle, m_allocator, &m_handle
    ));
}

void VKSwapchain::createImages() {
    VK_ASSERT(vkGetSwapchainImagesKHR(m_device, m_handle, &m_imageCount, 0));
    ASSERT(m_imageCount > 0, "swapchainImageCount==0 for vulkan swapchain");

    std::vector<VkImage> swapchainImages(m_imageCount, 0);
    VK_ASSERT(vkGetSwapchainImagesKHR(
      m_device, m_handle, &m_imageCount, swapchainImages.data()
    ));

    auto samplerProperties = Texture::SamplerProperties::createDefault();

    if (m_textures.size() != m_imageCount) {
        m_textures.clear();
        m_textures.resize(m_imageCount);

        auto imageData = Texture::ImageData::createDefault();

        imageData.width    = m_swapchainExtent.width;
        imageData.height   = m_swapchainExtent.height;
        imageData.channels = 4;
        imageData.flags    = Texture::Flags::writable;
        imageData.format   = static_cast<Format>(m_imageFormat.format);

        for (u32 i = 0; i < m_imageCount; ++i) {
            auto& swapchainImageHandle = swapchainImages[i];

            m_textures[i].emplace(
              m_device, m_allocator, swapchainImageHandle, imageData,
              samplerProperties
            );
        }
    } else {
        for (u32 i = 0; i < m_imageCount; ++i) {
            auto& texture = m_textures[i];
            texture->resize(m_swapchainExtent.width, m_swapchainExtent.height);
        }
    }

    auto imageData     = Texture::ImageData::createDefault();
    imageData.width    = m_swapchainExtent.width;
    imageData.height   = m_swapchainExtent.height;
    imageData.usage    = Texture::Usage::depthStencilAttachment;
    imageData.aspect   = Texture::Aspect::depth;
    imageData.format   = static_cast<Format>(m_depthFormat);
    imageData.channels = m_depthChannels;

    m_depthTexture.emplace(m_device, m_allocator, imageData, samplerProperties);
}

void VKSwapchain::create() {
    createSwapchain();
    createImages();
    LOG_INFO("VKSwapchain created successfully.");
}

VKSwapchain::~VKSwapchain() { destroy(); }

void VKSwapchain::destroy() {
    vkDestroySwapchainKHR(m_device, m_handle, m_allocator);
    m_textures.clear();
    m_depthTexture.clear();
    LOG_TRACE("VKSwapchain destroyed");
}

std::optional<u32> VKSwapchain::acquireNextImageIndex(
  Semaphore* imageSemaphore, Fence* fence, Nanoseconds timeout
) {
    u32 index = 0;

    VkResult result = vkAcquireNextImageKHR(
      m_device, m_handle, timeout,
      imageSemaphore != nullptr ? toVk(*imageSemaphore).getHandle() : nullptr,
      fence != nullptr ? toVk(*fence).getHandle() : nullptr, &index
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        LOG_WARN("Swapchain out of date, recreating");
        destroy();
        create();
        return {};
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOG_ERROR(
          "Failed to acquire swapchain image: {}", getResultString(result, true)
        );
        return {};
    }

    return index;
}

void VKSwapchain::recreate(const Vec2<u32>& size) {
    LOG_INFO("Recreating swapchain: {}/{}", size.w, size.h);

    m_size = size;
    destroy();
    create();
}

u32 VKSwapchain::getImageCount() const { return m_imageCount; }

VKSwapchain& toVk(Swapchain& swapchain) {
    return static_cast<VKSwapchain&>(swapchain);
}

}  // namespace sl::vk
