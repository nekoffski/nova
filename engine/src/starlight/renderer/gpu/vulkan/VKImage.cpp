#include "VKImage.hh"

#include "VKContext.hh"

namespace sl::vk {

VKImage::VKImage(
  VKContext& context, VKLogicalDevice& device, const VKImage::Properties& properties
) :
    m_context(context), m_device(device), m_props(properties),
    m_handle(VK_NULL_HANDLE), m_memory(VK_NULL_HANDLE), m_view(VK_NULL_HANDLE),
    m_destroyImage(true) {
    create();
}

VKImage::VKImage(
  VKContext& context, VKLogicalDevice& device, const Properties& properties,
  std::span<const u8> pixels
) : VKImage(context, device, properties) {
    write(0, pixels);
}

VKImage::VKImage(
  VKContext& context, VKLogicalDevice& device, const Properties& properties,
  VkImage handle
) :
    m_context(context), m_device(device), m_props(properties), m_handle(handle),
    m_memory(VK_NULL_HANDLE), m_view(VK_NULL_HANDLE), m_destroyImage(false) {
    if (m_props.createView) createView();
}

void VKImage::destroy() {
    auto logicalDeviceHandle = m_device.getHandle();
    auto allocator           = m_context.getAllocator();

    if (m_view) vkDestroyImageView(logicalDeviceHandle, m_view, allocator);
    if (m_memory) vkFreeMemory(logicalDeviceHandle, m_memory, allocator);
    if (m_handle && m_destroyImage)
        vkDestroyImage(logicalDeviceHandle, m_handle, allocator);
    LOG_TRACE("VKImage destroyed");
}

const VKImage::Properties& VKImage::getProperties() const { return m_props; }

void VKImage::recreate(const Properties& properties, VkImage handle) {
    destroy();

    m_props        = properties;
    m_destroyImage = false;
    m_handle       = handle;

    if (m_props.createView) createView();
}

void VKImage::recreate(const Properties& properties) {
    destroy();
    m_props = properties;
    create();
}

void VKImage::create() {
    createImage();
    allocateAndBindMemory();

    if (m_props.createView) createView();
}

VKImage::~VKImage() { destroy(); }

void VKImage::write([[maybe_unused]] u32 offset, std::span<const u8> pixels) {
    VkDeviceSize imageSize = pixels.size();

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memoryProps =
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VKBuffer::Properties stagingBufferProperties{
        .size                = imageSize,
        .memoryPropertyFlags = memoryProps,
        .usageFlags          = usage,
        .bindOnCreate        = true,
        .useFreeList         = false
    };

    VKBuffer stagingBuffer(m_context, m_device, stagingBufferProperties);

    stagingBuffer.loadData(0, imageSize, 0, pixels.data());

    VKCommandBuffer tempCommandBuffer{ m_device, m_device.getGraphicsCommandPool() };
    VkQueue graphicsQueue = m_device.getQueues().graphics;

    tempCommandBuffer.createAndBeginSingleUse();

    transitionLayout(
      tempCommandBuffer, m_props.format, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    copyFromBuffer(stagingBuffer, tempCommandBuffer);
    transitionLayout(
      tempCommandBuffer, m_props.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    tempCommandBuffer.endSingleUse(graphicsQueue);
}

void VKImage::copyFromBuffer(VKBuffer& buffer, VKCommandBuffer& commandBuffer) {
    VkBufferImageCopy region;
    std::memset(&region, 0, sizeof(VkBufferImageCopy));

    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = m_props.type == Type::cubemap ? 6 : 1;

    region.imageExtent.width  = m_props.width;
    region.imageExtent.height = m_props.height;
    region.imageExtent.depth  = 1;

    LOG_TRACE("vkCmdCopyBufferToImage region: {}/{}", m_props.width, m_props.height);

    vkCmdCopyBufferToImage(
      commandBuffer.getHandle(), buffer.getHandle(), m_handle,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region
    );
}

void VKImage::transitionLayout(
  VKCommandBuffer& commandBuffer, [[maybe_unused]] VkFormat format,
  VkImageLayout oldLayout, VkImageLayout newLayout
) {
    auto queueFamilyIndex = m_device.getQueueIndices().graphics;

    VkImageMemoryBarrier barrier;
    clearMemory(&barrier);
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = queueFamilyIndex;
    barrier.dstQueueFamilyIndex             = queueFamilyIndex;
    barrier.image                           = m_handle;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = m_props.type == Type::cubemap ? 6 : 1;

    VkPipelineStageFlags source;
    VkPipelineStageFlags destination;

    // Don't care about the old layout - transition to optimal layout (for the
    // underlying implementation).
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
        && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        // Don't care what stage the pipeline is in at the start.
        source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        // Used for copying
        destination = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
               && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        // Transitioning from a transfer destination layout to a shader-readonly
        // layout.
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // From a copying stage to...
        source = VK_PIPELINE_STAGE_TRANSFER_BIT;

        // The fragment stage.
        destination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        LOG_ERROR("Unsupported layout transition");
        return;
    }

    vkCmdPipelineBarrier(
      commandBuffer.getHandle(), source, destination, 0, 0, 0, 0, 0, 1, &barrier
    );
}

VkImageCreateInfo createImageCreateInfo(const VKImage::Properties& properties) {
    bool isCubemap = properties.type == VKImage::Type::cubemap;

    VkImageCreateInfo imageCreateInfo;
    imageCreateInfo.flags         = 0;
    imageCreateInfo.pNext         = nullptr;
    imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width  = properties.width;
    imageCreateInfo.extent.height = properties.height;
    imageCreateInfo.extent.depth  = 1;  // TODO: Support configurable depth.
    imageCreateInfo.mipLevels     = 4;  // TODO: Support mip mapping
    imageCreateInfo.arrayLayers   = isCubemap ? 6 : 1;
    imageCreateInfo.format        = properties.format;
    imageCreateInfo.tiling        = properties.tiling;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage         = properties.usage;
    imageCreateInfo.samples =
      VK_SAMPLE_COUNT_1_BIT;  // TODO: Configurable sample count.
    imageCreateInfo.sharingMode =
      VK_SHARING_MODE_EXCLUSIVE;  // TODO: Configurable sharing mode.
    if (isCubemap) imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    return imageCreateInfo;
}

VkImageViewCreateInfo createViewCreateInfo(
  const VKImage::Properties& properties, VkImage imageHandle
) {
    VkImageViewCreateInfo viewCreateInfo;
    clearMemory(&viewCreateInfo);
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    bool isCubemap = properties.type == VKImage::Type::cubemap;

    viewCreateInfo.image = imageHandle;
    viewCreateInfo.viewType =
      isCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format                          = properties.format;
    viewCreateInfo.subresourceRange.aspectMask     = properties.viewAspectFlags;
    viewCreateInfo.subresourceRange.baseMipLevel   = 0;
    viewCreateInfo.subresourceRange.levelCount     = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount     = isCubemap ? 6 : 1;
    viewCreateInfo.flags                           = 0;

    return viewCreateInfo;
}

VkMemoryRequirements getMemoryRequirements(VkDevice device, VkImage handle) {
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, handle, &memoryRequirements);

    return memoryRequirements;
}

VkMemoryAllocateInfo createMemoryAllocateInfo(
  const VkMemoryRequirements& requirements, uint32_t memoryTypeIndex
) {
    VkMemoryAllocateInfo memoryAllocateInfo;
    memoryAllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize  = requirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
    memoryAllocateInfo.pNext           = nullptr;

    return memoryAllocateInfo;
}

VkImageView VKImage::getView() const { return m_view; }

void VKImage::createImage(

) {
    auto imageCreateInfo = createImageCreateInfo(m_props);
    VK_ASSERT(vkCreateImage(
      m_device.getHandle(), &imageCreateInfo, m_context.getAllocator(), &m_handle
    ));
    m_destroyImage = true;
    LOG_TRACE("VKImage created");
}

void VKImage::allocateAndBindMemory() {
    auto logicalDeviceHandle = m_device.getHandle();
    auto memoryRequirements  = getMemoryRequirements(logicalDeviceHandle, m_handle);

    auto memoryType = m_device.findMemoryIndex(
      memoryRequirements.memoryTypeBits, m_props.memoryFlags
    );

    if (not memoryType)
        LOG_ERROR("Required memory type not found. VKImage not valid.");

    auto memoryAllocateInfo =
      createMemoryAllocateInfo(memoryRequirements, memoryType.value_or(-1));
    VK_ASSERT(vkAllocateMemory(
      logicalDeviceHandle, &memoryAllocateInfo, m_context.getAllocator(), &m_memory
    ));

    VK_ASSERT(vkBindImageMemory(logicalDeviceHandle, m_handle, m_memory, 0));
}

void VKImage::createView(

) {
    auto viewCreateInfo = createViewCreateInfo(m_props, m_handle);
    VK_ASSERT(vkCreateImageView(
      m_device.getHandle(), &viewCreateInfo, m_context.getAllocator(), &m_view
    ));
}

}  // namespace sl::vk
