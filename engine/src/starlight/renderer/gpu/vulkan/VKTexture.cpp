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

VkFormat channelsToFormat(u32 channels) {
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

VkSamplerCreateInfo createSamplerCreateInfo(const Texture::Properties& props) {
    // clang-format off
    static std::unordered_map<Texture::Repeat, VkSamplerAddressMode> vkRepeat{
        {Texture::Repeat::repeat,          VK_SAMPLER_ADDRESS_MODE_REPEAT},
        { Texture::Repeat::mirroredRepeat,
        VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT}, {
        Texture::Repeat::clampToEdge, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE},
        { Texture::Repeat::clampToBorder,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER},
    };
    // clang-format on
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

VKImage::Properties getImageProperties(
  u32 width, u32 height, Texture::Type textureType,
  VkFormat format = VK_FORMAT_R8G8B8A8_UNORM
) {
    return VKImage::Properties{
        textureType == Texture::Type::cubemap
          ? VKImage::Type::cubemap
          : VKImage::Type::flat,
        width,
        height,
        format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
          | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
          | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_COLOR_BIT,
        3  // TODO: how to get those channels?
    };
}

}  // namespace

VKTexture::VKTexture(
  VKContext& context, VKLogicalDevice& device, const Properties& props,
  std::span<const u8> pixels
) :
    Texture(props), m_context(context), m_device(device),
    m_image(
      m_context, m_device,
      getImageProperties(
        props.width, props.height, props.type, channelsToFormat(props.channels)
      ),
      pixels
    ),
    m_generation(1u) {
    LOG_TRACE("Texture created");
    createSampler(props);
}

VKTexture::VKTexture(
  VKContext& context, VKLogicalDevice& device, const Properties& props,
  VkImage handle, VkFormat format
) :
    Texture(props), m_context(context), m_device(device),
    m_image(
      m_context, m_device,
      getImageProperties(props.width, props.height, props.type, format), handle
    ),
    m_generation(1u) {
    LOG_TRACE("Texture created from existing VKImage");
    createSampler(props);
}

VKTexture::VKTexture(
  VKContext& context, VKLogicalDevice& device, const VKImage::Properties& props
) :
    Texture(Properties{
      props.width,
      props.height,
      props.channels,
      true,
      false,
      Type::flat,
      Filter::linear,
      Filter::linear,
      Repeat::repeat,
      Repeat::repeat,
      Repeat::repeat,
    }),
    m_context(context), m_device(device), m_image(m_context, m_device, props) {
    createSampler(m_props);
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
    m_props.width  = width;
    m_props.height = height;

    m_image.recreate(getImageProperties(
      width, height, m_props.type, channelsToFormat(m_props.channels)
    ));
}

void VKTexture::resize(u32 width, u32 height, VkImage handle) {
    m_props.width  = width;
    m_props.height = height;

    auto props       = m_image.getProperties();
    props.width      = width;
    props.height     = height;
    props.createView = true;

    m_image.recreate(props, handle);
}

void VKTexture::write(u32 offset, std::span<u8> pixels) {
    m_image.write(offset, pixels);
}

void VKTexture::createSampler(const Texture::Properties& props) {
    const auto samplerInfo = createSamplerCreateInfo(props);
    VK_ASSERT(vkCreateSampler(
      m_device.getHandle(), &samplerInfo, m_context.getAllocator(), &m_sampler
    ));
}

}  // namespace sl::vk
