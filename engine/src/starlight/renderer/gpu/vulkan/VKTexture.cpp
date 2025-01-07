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
namespace {

VkSamplerCreateInfo createSamplerCreateInfo(const Texture::SamplerProperties& props
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

}  // namespace

VKTexture::VKTexture(
  VKContext& context, VKLogicalDevice& device, const ImageData& imageData,
  const SamplerProperties& sampler
) :
    Texture(imageData, sampler), m_context(context), m_device(device),
    m_image(m_context, m_device, imageData), m_generation(1u) {
    LOG_TRACE("Texture created");
    createSampler(sampler);
}

VKTexture::VKTexture(
  VKContext& context, VKLogicalDevice& device, VkImage handle,
  const ImageData& imageData, const SamplerProperties& sampler
) :
    Texture(imageData, sampler), m_context(context), m_device(device),
    m_image(m_context, m_device, imageData, handle), m_generation(1u) {
    LOG_TRACE("Texture created from existing VKImage");
    createSampler(sampler);
}

VKTexture::~VKTexture() {
    const auto logicalDeviceHandle = m_device.getHandle();

    vkDeviceWaitIdle(logicalDeviceHandle);
    vkDestroySampler(logicalDeviceHandle, m_sampler, m_context.getAllocator());
    LOG_TRACE("Texture destroyed");
}

const VKImage* VKTexture::getImage() const { return &m_image; }

VkSampler VKTexture::getSampler() const { return m_sampler; }

void VKTexture::resize(u32 width, u32 height) {
    m_imageData.width  = width;
    m_imageData.height = height;
    m_image.recreate(m_imageData);
}

void VKTexture::resize(u32 width, u32 height, VkImage handle) {
    m_imageData.width  = width;
    m_imageData.height = height;
    m_image.recreate(m_imageData, handle);
}

void VKTexture::write(u32 offset, std::span<u8> pixels) {
    m_image.write(offset, pixels);
}

void VKTexture::createSampler(const Texture::SamplerProperties& props) {
    const auto samplerInfo = createSamplerCreateInfo(props);
    VK_ASSERT(vkCreateSampler(
      m_device.getHandle(), &samplerInfo, m_context.getAllocator(), &m_sampler
    ));
}

}  // namespace sl::vk
