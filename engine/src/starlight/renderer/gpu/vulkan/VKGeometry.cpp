#include "VKGeometry.h"

namespace sl::vk {

VKGeometry::VKGeometry(
  u32 id, VKDevice* device, VKContext* context, VKBuffer& vertexBuffer,
  VKBuffer& indexBuffer, const Properties& props, const Data& data
) :
    Geometry(props, data, id),
    m_device(device), m_context(context) {
    LOG_TRACE("Creating Geometry");
    LOG_DEBUG(
      "Geometry data vertexSize={}, vertexCount={}, "
      "indicesCount={}, vertexDataPtr={}",
      data.vertexSize, data.vertexCount, data.indices.size(), data.vertexData
    );

    m_dataDescription.vertexCount        = data.vertexCount;
    m_dataDescription.vertexElementSize  = data.vertexSize;
    m_dataDescription.indexCount         = data.indices.size();
    m_dataDescription.indexElementSize   = sizeof(u32);
    m_dataDescription.vertexBufferOffset = 0;
    m_dataDescription.indexBufferOffset  = 0;

    upload(vertexBuffer, indexBuffer, data);
    LOG_TRACE("Geometry created");
}

VKGeometry::~VKGeometry() {
    LOG_TRACE("Destroying geometry");
    // TODO: fill
    m_device->waitIdle();
}

u64 VKGeometry::uploadDataRange(
  VkCommandPool pool, VkFence fence, VkQueue queue, VKBuffer& outBuffer, u64 size,
  const void* data
) {
    // TODO: shouldn't it be a part of VKBuffer class?
    const auto offset = outBuffer.allocate(size);

    VkMemoryPropertyFlags flags =
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VKBuffer stagingBuffer(
      m_context, m_device,
      VKBuffer::Properties{ size, flags, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true }
    );

    stagingBuffer.loadData(0, size, 0, data);
    stagingBuffer.copyTo(
      pool, fence, queue, outBuffer.getHandle(),
      VkBufferCopy{ .srcOffset = 0, .dstOffset = offset, .size = size }
    );
    // this should return optional in case if allocate returns false
    return offset;
}

void VKGeometry::upload(
  VKBuffer& vertexBuffer, VKBuffer& indexBuffer, const Data& data
) {
    auto pool  = m_device->getGraphicsCommandPool();
    auto queue = m_device->getQueues().graphics;

    m_dataDescription.vertexBufferOffset = uploadDataRange(
      pool, nullptr, queue, vertexBuffer, m_dataDescription.verticesTotalSize(),
      data.vertexData
    );

    // TODO: allow for indices to be optional
    if (m_dataDescription.indexCount > 0) {
        m_dataDescription.indexBufferOffset = uploadDataRange(
          pool, nullptr, queue, indexBuffer, m_dataDescription.indicesTotalSize(),
          data.indices.data()
        );
    }
}

}  // namespace sl::vk