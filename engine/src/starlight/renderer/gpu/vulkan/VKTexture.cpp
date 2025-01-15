#include "VKTexture.hh"

#include <string>
#include <vector>

#include <stb.h>

#include <kc/core/Log.h>
#include <kc/core/Scope.hpp>

#include "Vulkan.hh"
#include "VKImage.hh"
#include "VKBuffer.hh"
#include "VKContext.hh"
#include "VKCommandBuffer.hh"

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

void VKTextureBase::createSampler() {
    const auto samplerInfo = createSamplerCreateInfo(m_samplerProperties);
    VK_ASSERT(vkCreateSampler(m_device, &samplerInfo, m_allocator, &m_sampler));
}

VKTextureBase::VKTextureBase(
  VkDevice device, Allocator* allocator, const ImageData& imageData,
  const SamplerProperties& sampler
) :
    Texture(imageData, sampler), m_device(device), m_allocator(allocator),
    m_image(VK_NULL_HANDLE), m_sampler(VK_NULL_HANDLE), m_view(VK_NULL_HANDLE) {}

VkImageView VKTextureBase::getView() const { return m_view; }

VkSampler VKTextureBase::getSampler() const { return m_sampler; }

/*

    VKTexture

*/

VKTexture::VKTexture(
  VkDevice device, Allocator* allocator, const ImageData& imageData,
  const SamplerProperties& sampler
) : VKTextureBase(device, allocator, imageData, sampler) {
    LOG_TRACE("Texture created");
    createSampler();
}

VKTexture::~VKTexture() {
    vkDestroySampler(m_device, m_sampler, m_allocator);
    LOG_TRACE("Texture destroyed");
}

void VKTexture::resize(u32 width, u32 height) {
    // m_imageData.width  = width;
    // m_imageData.height = height;
    // m_image.recreate(m_imageData);
}

void VKTexture::write(std::span<u8> pixels) {
    // m_image.write(pixels);
}

/*

    VKSwapchainTexture

*/

VKSwapchainTexture::VKSwapchainTexture(
  VkDevice device, Allocator* allocator, VkImage handle, const ImageData& imageData,
  const SamplerProperties& sampler
) : VKTextureBase(device, allocator, imageData, sampler) {
    m_image = handle;
    createSampler();
    LOG_TRACE("Swapchain texture created");
}

VKSwapchainTexture::~VKSwapchainTexture() {}

void VKSwapchainTexture::resize(u32 width, u32 height) {
    LOG_ERROR("Cannot resize swapchain texture");
}

void VKSwapchainTexture::write(std::span<u8> pixels) {
    LOG_ERROR("Cannot write to swapchain texture");
}

}  // namespace sl::vk
