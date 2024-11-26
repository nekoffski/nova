#pragma once

#include <span>

#include "VKPhysicalDevice.hh"
#include "VKContext.hh"
#include "VKBuffer.hh"
#include "starlight/renderer/gpu/Mesh.hh"

namespace sl::vk {

static constexpr int vulkanMaxMeshCount = 4096;

class VKMesh : public Mesh {
public:
    explicit VKMesh(
      VKContext& context, VKLogicalDevice& device, VKBuffer& vertexBuffer,
      VKBuffer& indexBuffer, const Data& data
    );
    ~VKMesh();

private:
    void upload(VKBuffer& vertexBuffer, VKBuffer& indexBuffer, const Data& data);

    u64 uploadDataRange(
      VkCommandPool pool, VkFence fence, VkQueue queue, VKBuffer& outBuffer,
      u64 size, const void* data
    );

    VKContext& m_context;
    VKLogicalDevice& m_device;
};

}  // namespace sl::vk
