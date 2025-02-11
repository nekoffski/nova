#include "VulkanTexture.hh"

#include <string>
#include <vector>

#include <stb.h>

#include "starlight/core/Scope.hh"

#include "Vulkan.hh"
#include "VulkanBuffer.hh"
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

    VulkanTextureBase

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

void VulkanTextureBase::createSampler() {
    const auto samplerInfo = createSamplerCreateInfo(m_samplerProperties);
    log::expect(vkCreateSampler(
      m_device.logical.handle, &samplerInfo, m_device.allocator, &m_sampler
    ));
    log::trace("vkCreateSampler: {}", static_cast<void*>(m_sampler));
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

void VulkanTextureBase::createView() {
    auto viewCreateInfo = createViewCreateInfo(m_imageData, m_image);
    log::expect(vkCreateImageView(
      m_device.logical.handle, &viewCreateInfo, m_device.allocator, &m_view
    ));
    log::trace("vkCreateImageView: {}", static_cast<void*>(m_view));
}

VulkanTextureBase::VulkanTextureBase(
  VulkanDevice& device, const ImageData& imageData, const SamplerProperties& sampler
) :
    Texture(imageData, sampler), m_device(device), m_image(VK_NULL_HANDLE),
    m_sampler(VK_NULL_HANDLE), m_view(VK_NULL_HANDLE) {}

VkImageView VulkanTextureBase::getView() const { return m_view; }

VkSampler VulkanTextureBase::getSampler() const { return m_sampler; }

/*

    VulkanTexture

*/

VulkanTexture::VulkanTexture(
  VulkanDevice& device, const ImageData& imageData, const SamplerProperties& sampler
) :
    VulkanTextureBase(device, imageData, sampler), m_memory(VK_NULL_HANDLE),
    m_layout(VK_IMAGE_LAYOUT_GENERAL) {
    log::trace("Creating vulkan texture: {}", getId());
    create();
}

VulkanTexture::~VulkanTexture() { destroy(); }

void VulkanTexture::resize(u32 width, u32 height) {
    m_imageData.width  = width;
    m_imageData.height = height;
    recreate(m_imageData);
}

void VulkanTexture::copyFromBuffer(
  VulkanBuffer& buffer, VulkanCommandBuffer& commandBuffer
) {
    VkBufferImageCopy region;
    std::memset(&region, 0, sizeof(VkBufferImageCopy));

    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount =
      m_imageData.type == Texture::Type::cubemap ? 6 : 1;

    region.imageExtent.width  = m_imageData.width;
    region.imageExtent.height = m_imageData.height;
    region.imageExtent.depth  = 1;

    vkCmdCopyBufferToImage(
      commandBuffer.getHandle(), buffer.getHandle(), m_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region
    );
}

void VulkanTexture::transitionLayout(
  VulkanCommandBuffer& commandBuffer, VkImageLayout oldLayout,
  VkImageLayout newLayout
) {
    auto queueFamilyIndex =
      m_device.physical.info.queueIndices.at(Queue::Type::graphics);

    VkImageMemoryBarrier barrier;
    clearMemory(&barrier);
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = queueFamilyIndex;
    barrier.dstQueueFamilyIndex             = queueFamilyIndex;
    barrier.image                           = m_image;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount =
      m_imageData.type == Texture::Type::cubemap ? 6 : 1;

    VkPipelineStageFlags source;
    VkPipelineStageFlags destination;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
        && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source                = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination           = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
               && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source                = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination           = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        log::error("Unsupported layout transition");
        return;
    }
    vkCmdPipelineBarrier(
      commandBuffer.getHandle(), source, destination, 0, 0, 0, 0, 0, 1, &barrier
    );
}

void VulkanTexture::write(std::span<u8> pixels, CommandBuffer* commandBuffer) {
    const auto imageSize = pixels.size();

    const auto memoryProps =
      MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT
      | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT;

    Buffer::Properties stagingBufferProperties{
        .size           = imageSize,
        .memoryProperty = memoryProps,
        .usage          = BufferUsage::BUFFER_USAGE_TRANSFER_SRC_BIT,
        .bindOnCreate   = true
    };

    VulkanBuffer stagingBuffer(m_device, stagingBufferProperties);
    stagingBuffer.copy(Range{ .offset = 0u, .size = imageSize }, pixels.data());

    const auto execute = [&](CommandBuffer& commandBuffer) {
        auto& vkCommandBuffer = static_cast<VulkanCommandBuffer&>(commandBuffer);

        transitionLayout(
          vkCommandBuffer, VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );

        copyFromBuffer(stagingBuffer, vkCommandBuffer);

        transitionLayout(
          vkCommandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
    };

    if (commandBuffer != nullptr) {
        execute(*commandBuffer);
    } else {
        ImmediateCommandBuffer commandBuffer{
            m_device.createCommandBuffer(), m_device.getGraphicsQueue()
        };
        execute(commandBuffer.get());
    }
    m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

VkImageLayout VulkanTexture::getLayout() const { return m_layout; }

void VulkanTexture::create() {
    createImage();
    allocateAndBindMemory();
    if (m_imageData.pixels.size() > 0) write(m_imageData.pixels);
    createView();
    createSampler();
}

void VulkanTexture::destroy() {
    log::trace("Destroying vulkan texture: {}", getId());
    auto device    = m_device.logical.handle;
    auto allocator = m_device.allocator;
    m_device.waitIdle();

    if (m_sampler) {
        log::trace("vkDestroySampler: {}", static_cast<void*>(m_sampler));
        vkDestroySampler(device, m_sampler, allocator);
    }

    if (m_view) {
        log::trace("vkDestroyImageView: {}", static_cast<void*>(m_view));
        vkDestroyImageView(device, m_view, allocator);
    }

    if (m_memory) {
        log::trace("vkFreeMemory: {}", static_cast<void*>(m_memory));
        vkFreeMemory(device, m_memory, allocator);
    }

    if (m_image) {
        log::trace("vkDestroyImage: {}", static_cast<void*>(m_image));
        vkDestroyImage(device, m_image, allocator);
    }
}

void VulkanTexture::allocateAndBindMemory() {
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(
      m_device.logical.handle, m_image, &memoryRequirements
    );

    auto memoryType = m_device.findMemoryIndex(
      memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    if (not memoryType)
        log::error("Required memory type not found. VKImage not valid.");

    VkMemoryAllocateInfo memoryAllocateInfo;
    clearMemory(&memoryAllocateInfo);
    memoryAllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize  = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryType.value_or(-1);

    log::expect(vkAllocateMemory(
      m_device.logical.handle, &memoryAllocateInfo, m_device.allocator, &m_memory
    ));
    log::trace("vkAllocateMemory: {}", static_cast<void*>(m_memory));
    log::expect(vkBindImageMemory(m_device.logical.handle, m_image, m_memory, 0));
}

void VulkanTexture::recreate(const Texture::ImageData& imageData) {
    destroy();
    m_imageData = imageData;
    create();
}

void VulkanTexture::createImage() {
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

    log::expect(vkCreateImage(
      m_device.logical.handle, &imageCreateInfo, m_device.allocator, &m_image
    ));
    log::trace("vkCreateImage: {}", static_cast<void*>(m_image));
}

/*

    VulkanSwapchainTexture

*/

VulkanSwapchainTexture::VulkanSwapchainTexture(
  VulkanDevice& device, VkImage handle, const ImageData& imageData,
  const SamplerProperties& sampler
) : VulkanTextureBase(device, imageData, sampler) {
    m_image = handle;
    createView();
    createSampler();
    log::trace("Swapchain texture created");
}

VulkanSwapchainTexture::~VulkanSwapchainTexture() {
    if (m_sampler)
        vkDestroySampler(m_device.logical.handle, m_sampler, m_device.allocator);
    if (m_view)
        vkDestroyImageView(m_device.logical.handle, m_view, m_device.allocator);
}

void VulkanSwapchainTexture::resize(
  [[maybe_unused]] u32 width, [[maybe_unused]] u32 height
) {
    log::error("Cannot resize swapchain texture");
}

void VulkanSwapchainTexture::write(
  [[maybe_unused]] std::span<u8> pixels, [[maybe_unused]] CommandBuffer* buffer
) {
    log::error("Cannot write to swapchain texture");
}

}  // namespace sl::vk
