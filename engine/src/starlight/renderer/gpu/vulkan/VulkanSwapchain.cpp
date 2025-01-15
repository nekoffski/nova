#include "VulkanSwapchain.hh"

#include <kc/core/Log.h>
#include <kc/core/Utils.hpp>

#include "VKFramebuffer.hh"
#include "VulkanSemaphore.hh"
#include "VulkanDevice.hh"
#include "VulkanFence.hh"

namespace sl::vk {

VulkanSwapchain::VulkanSwapchain(VulkanDevice& device, const Vec2<u32>& size) :
    m_handle(VK_NULL_HANDLE), m_device(device), m_size(size) {
    create();
}

VkSurfaceFormatKHR pickSurfaceFormat(const VulkanDevice::Physical::Info& deviceInfo
) {
    static const auto demandedFormat     = VK_FORMAT_R8G8B8A8_UNORM;
    static const auto demandedColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    for (const auto& format : deviceInfo.surfaceFormats)
        if (format.format == demandedFormat
            && format.colorSpace == demandedColorSpace) {
            return format;
        }
    return deviceInfo.surfaceFormats[0];
}

VkPresentModeKHR pickPresentMode(const VulkanDevice::Physical::Info& deviceInfo) {
    static const auto defaultPresentMode  = VK_PRESENT_MODE_FIFO_KHR;
    static const auto demandedPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

    if (kc::core::contains(deviceInfo.presentModes, demandedPresentMode))
        return demandedPresentMode;
    return defaultPresentMode;
}

VkExtent2D createSwapchainExtent(
  u32 viewportWidth, u32 viewportHeight,
  const VulkanDevice::Physical::Info& deviceInfo
) {
    VkExtent2D swapchainExtent = { viewportWidth, viewportHeight };

    const auto min = deviceInfo.surfaceCapabilities.minImageExtent;
    const auto max = deviceInfo.surfaceCapabilities.maxImageExtent;

    LOG_INFO(
      "Swapchain capabilities min/max - {} - {}/{} - {}", min.width, min.height,
      max.width, max.height
    );

    swapchainExtent.height =
      std::clamp(swapchainExtent.height, min.height, max.height);

    return swapchainExtent;
}

Texture* VulkanSwapchain::getImage(u32 id) {
    ASSERT(id < m_textures.size(), "Invalid image id - {}", id);
    return m_textures[id].get();
}

Texture* VulkanSwapchain::getDepthBuffer() { return m_depthTexture.get(); }

static u8 getDeviceImageCount(const VulkanDevice::Physical::Info& deviceInfo) {
    auto imageCount = deviceInfo.surfaceCapabilities.minImageCount + 1;

    if (deviceInfo.surfaceCapabilities.maxImageCount > 0
        && imageCount > deviceInfo.surfaceCapabilities.maxImageCount) {
        imageCount = deviceInfo.surfaceCapabilities.maxImageCount;
    }

    return imageCount;
}

VkSurfaceFormatKHR VulkanSwapchain::getSurfaceFormat() const {
    return m_imageFormat;
}

VkSwapchainKHR* VulkanSwapchain::getHandlePtr() { return &m_handle; }

struct SwapchainCreateInfo {
    explicit SwapchainCreateInfo() { clearMemory(&handle); }

    VkSwapchainCreateInfoKHR handle;
    std::vector<u32> queueFamilyIndices;
};

SwapchainCreateInfo createSwapchainCreateInfo(
  VkSurfaceKHR surface, u32 imageCount, VkSurfaceFormatKHR imageFormat,
  VkExtent2D extent, VkPresentModeKHR presentMode,
  const VulkanDevice::Physical::Info& deviceInfo
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

    auto graphicsQueue = deviceInfo.queueIndices.at(Queue::Type::graphics);
    auto presentQueue  = deviceInfo.queueIndices.at(Queue::Type::present);

    createInfo.queueFamilyIndices = { graphicsQueue, presentQueue };

    if (graphicsQueue != presentQueue) {
        handle.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        handle.queueFamilyIndexCount = 2;
        handle.pQueueFamilyIndices   = createInfo.queueFamilyIndices.data();
    } else {
        handle.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        handle.queueFamilyIndexCount = 0;
        handle.pQueueFamilyIndices   = 0;
    }

    handle.preTransform   = deviceInfo.surfaceCapabilities.currentTransform;
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

void VulkanSwapchain::createSwapchain() {
    m_imageFormat    = pickSurfaceFormat(m_device.physical.info);
    auto presentMode = pickPresentMode(m_device.physical.info);

    m_swapchainExtent =
      createSwapchainExtent(m_size.w, m_size.h, m_device.physical.info);
    auto deviceImageCount = getDeviceImageCount(m_device.physical.info);

    LOG_INFO(
      "Creating swapchain: {}/{}", m_swapchainExtent.width, m_swapchainExtent.height
    );

    auto swapchainCreateInfo = createSwapchainCreateInfo(
      m_device.surface.handle, deviceImageCount, m_imageFormat, m_swapchainExtent,
      presentMode, m_device.physical.info
    );

    VK_ASSERT(vkCreateSwapchainKHR(
      m_device.logical.handle, &swapchainCreateInfo.handle, m_device.allocator,
      &m_handle
    ));
}

void VulkanSwapchain::createImages() {
    VK_ASSERT(
      vkGetSwapchainImagesKHR(m_device.logical.handle, m_handle, &m_imageCount, 0)
    );
    ASSERT(m_imageCount > 0, "swapchainImageCount==0 for vulkan swapchain");

    std::vector<VkImage> swapchainImages(m_imageCount, 0);
    VK_ASSERT(vkGetSwapchainImagesKHR(
      m_device.logical.handle, m_handle, &m_imageCount, swapchainImages.data()
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
              m_device, swapchainImageHandle, imageData, samplerProperties
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
    imageData.format   = static_cast<Format>(m_device.physical.info.depthFormat);
    imageData.channels = m_device.physical.info.depthChannelCount;

    m_depthTexture.emplace(m_device, imageData, samplerProperties);
}

void VulkanSwapchain::create() {
    createSwapchain();
    createImages();
    LOG_INFO("VulkanSwapchain created successfully.");
}

VulkanSwapchain::~VulkanSwapchain() { destroy(); }

void VulkanSwapchain::destroy() {
    vkDestroySwapchainKHR(m_device.logical.handle, m_handle, m_device.allocator);
    m_textures.clear();
    m_depthTexture.clear();
    LOG_TRACE("VulkanSwapchain destroyed");
}

std::optional<u32> VulkanSwapchain::acquireNextImageIndex(
  Semaphore* imageSemaphore, Fence* fence, Nanoseconds timeout
) {
    u32 index = 0;

    VkResult result = vkAcquireNextImageKHR(
      m_device.logical.handle, m_handle, timeout,
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

void VulkanSwapchain::recreate(const Vec2<u32>& size) {
    LOG_INFO("Recreating swapchain: {}/{}", size.w, size.h);

    m_size = size;
    destroy();
    create();
}

u32 VulkanSwapchain::getImageCount() const { return m_imageCount; }

VulkanSwapchain& toVk(Swapchain& swapchain) {
    return static_cast<VulkanSwapchain&>(swapchain);
}

}  // namespace sl::vk
