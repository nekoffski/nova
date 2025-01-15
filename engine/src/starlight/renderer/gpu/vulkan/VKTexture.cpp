#include "VKTexture.hh"

#include <string>
#include <vector>

#include <stb.h>

#include <kc/core/Log.h>
#include <kc/core/Scope.hpp>

#include "Vulkan.hh"
#include "VKImage.hh"
#include "VulkanBuffer.hh"
#include "VKContext.hh"
#include "VulkanDevice.hh"
#include "VulkanCommandBuffer.hh"

namespace sl::vk {

static VkSamplerCreateInfo createSamplerCreateInfo(
  const Texture::SamplerProperties& props
) {
    static std::unordered_map<Texture::Repeat, VkSamplerAddressMode> vkRepeat{
        { Texture::Repeat::repeat,         VK_SAMPLER_ADDRESS_MODE_REPEAT          },
        { Texture::Repeat::mirroredRepeat, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT },
        { Texture::Repeat::clampToEdge,    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE   },
        { Texture::Repeat::clampToBorder,  VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER },
    };
    static std::unordered_map<Texture::Filter, VkFilter> vkFilter{
        { Texture::Filter::nearest, VK_FILTER_NEAREST },
        { Texture::Filter::linear,  VK_FILTER_LINEAR  }
    };

    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter               = vkFilter[props.magnifyFilter];
    samplerInfo.minFilter               = vkFilter[props.minifyFilter];
    samplerInfo.addressModeU            = vkRepeat[props.uRepeat];
    samplerInfo.addressModeV            = vkRepeat[props.vRepeat];
    samplerInfo.addressModeW            = vkRepeat[props.wRepeat];
    samplerInfo.anisotropyEnable        = VK_TRUE;
    samplerInfo.maxAnisotropy           = 16;
    samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias              = 0.0f;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = 0.0f;
    samplerInfo.pNext                   = nullptr;
    samplerInfo.flags                   = 0;

    return samplerInfo;
}

/*

    VKTextureBase

*/

static VkFormat channelsToFormat(u32 channels) {
    switch (channels) {
        case 1:
            return VK_FORMAT_R8_UNORM;
        case 2:
            return VK_FORMAT_R8G8_UNORM;
        case 3:
            return VK_FORMAT_R8G8B8_UNORM;
        case 4:
            return VK_FORMAT_R8G8B8A8_UNORM;
        default:
            return VK_FORMAT_R8G8B8A8_UNORM;
    }
}

static VkImageTiling toVk(Texture::Tiling tiling) {
    return static_cast<VkImageTiling>(tiling);
}

static VkImageAspectFlags toVk(Texture::Aspect aspect) {
    return static_cast<VkImageAspectFlags>(aspect);
}

static VkImageUsageFlags toVk(Texture::Usage usage) {
    return static_cast<VkImageUsageFlags>(usage);
}

static VkFormat toVk(Format format, u8 channels) {
    return format != Format::undefined
             ? static_cast<VkFormat>(format)
             : channelsToFormat(channels);
}

void VKTextureBase::createSampler() {
    const auto samplerInfo = createSamplerCreateInfo(m_samplerProperties);
    VK_ASSERT(vkCreateSampler(
      m_device.logical.handle, &samplerInfo, m_device.allocator, &m_sampler
    ));
}

static VkImageViewCreateInfo createViewCreateInfo(
  const Texture::ImageData& imageData, VkImage imageHandle
) {
    VkImageViewCreateInfo viewCreateInfo;
    clearMemory(&viewCreateInfo);
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    bool isCubemap = imageData.type == Texture::Type::cubemap;

    viewCreateInfo.image = imageHandle;
    viewCreateInfo.viewType =
      isCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = toVk(imageData.format, imageData.channels);
    viewCreateInfo.subresourceRange.aspectMask     = toVk(imageData.aspect);
    viewCreateInfo.subresourceRange.levelCount     = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount     = isCubemap ? 6 : 1;
    viewCreateInfo.flags                           = 0;

    return viewCreateInfo;
}

void VKTextureBase::createView() {
    auto viewCreateInfo = createViewCreateInfo(m_imageData, m_image);
    VK_ASSERT(vkCreateImageView(
      m_device.logical.handle, &viewCreateInfo, m_device.allocator, &m_view
    ))
}

VKTextureBase::VKTextureBase(
  VulkanDevice& device, const ImageData& imageData, const SamplerProperties& sampler
) :
    Texture(imageData, sampler), m_device(device), m_image(VK_NULL_HANDLE),
    m_sampler(VK_NULL_HANDLE), m_view(VK_NULL_HANDLE) {}

VkImageView VKTextureBase::getView() const { return m_view; }

VkSampler VKTextureBase::getSampler() const { return m_sampler; }

/*

    VKTexture

*/

VKTexture::VKTexture(
  VulkanDevice& device, const ImageData& imageData, const SamplerProperties& sampler
) : VKTextureBase(device, imageData, sampler), m_memory(VK_NULL_HANDLE) {
    create();
    LOG_TRACE("Texture created");
}

VKTexture::~VKTexture() {
    destroy();
    LOG_TRACE("Texture destroyed");
}

void VKTexture::resize(u32 width, u32 height) {
    m_imageData.width  = width;
    m_imageData.height = height;
    recreate(m_imageData);
}

void VKTexture::write(std::span<u8> pixels) {
    // m_image.write(pixels);
}

void VKTexture::create() {
    createImage();
    allocateAndBindMemory();
    if (m_imageData.pixels.size() > 0) write(m_imageData.pixels);
    createView();
    createSampler();
}

void VKTexture::destroy() {
    auto device    = m_device.logical.handle;
    auto allocator = m_device.allocator;

    if (m_sampler) vkDestroySampler(device, m_sampler, allocator);
    if (m_view) vkDestroyImageView(device, m_view, allocator);
    if (m_memory) vkFreeMemory(device, m_memory, allocator);
    if (m_image) vkDestroyImage(device, m_image, allocator);
    LOG_TRACE("VKImage destroyed");
}

void VKTexture::allocateAndBindMemory() {
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(
      m_device.logical.handle, m_image, &memoryRequirements
    );
}

void VKTexture::recreate(const Texture::ImageData& imageData) {
    destroy();
    m_imageData = imageData;
    create();
}

void VKTexture::createImage() {
    bool isCubemap = m_imageData.type == Texture::Type::cubemap;

    VkImageCreateInfo imageCreateInfo;
    clearMemory(&imageCreateInfo);
    imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width  = m_imageData.width;
    imageCreateInfo.extent.height = m_imageData.height;
    imageCreateInfo.extent.depth  = 1;
    imageCreateInfo.mipLevels     = 4;
    imageCreateInfo.arrayLayers   = isCubemap ? 6 : 1;
    imageCreateInfo.format        = toVk(m_imageData.format, m_imageData.channels);
    imageCreateInfo.tiling        = toVk(m_imageData.tiling);
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage         = toVk(m_imageData.usage);
    imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    if (isCubemap) imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VK_ASSERT(vkCreateImage(
      m_device.logical.handle, &imageCreateInfo, m_device.allocator, &m_image
    ));
}

/*

    VulkanSwapchainTexture

*/

VulkanSwapchainTexture::VulkanSwapchainTexture(
  VulkanDevice& device, VkImage handle, const ImageData& imageData,
  const SamplerProperties& sampler
) : VKTextureBase(device, imageData, sampler) {
    m_image = handle;
    createView();
    createSampler();
    LOG_TRACE("Swapchain texture created");
}

VulkanSwapchainTexture::~VulkanSwapchainTexture() {
    if (m_sampler)
        vkDestroySampler(m_device.logical.handle, m_sampler, m_device.allocator);
}

void VulkanSwapchainTexture::resize(u32 width, u32 height) {
    LOG_ERROR("Cannot resize swapchain texture");
}

void VulkanSwapchainTexture::write(std::span<u8> pixels) {
    LOG_ERROR("Cannot write to swapchain texture");
}

}  // namespace sl::vk
