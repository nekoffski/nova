// #pragma once

// #include <span>

// #include "VKPhysicalDevice.hh"
// #include "VKContext.hh"
// #include "VulkanBuffer.hh"
// #include "starlight/renderer/gpu/Mesh.hh"

// namespace sl::vk {

// static constexpr int vulkanMaxMeshCount = 4096;

// class VKMesh : public Mesh {
// public:
//     explicit VKMesh(
//       VKContext& context, VKLogicalDevice& device, VulkanBuffer& vertexBuffer,
//       VulkanBuffer& indexBuffer, const Data& data
//     );
//     ~VKMesh();

// private:
//     void upload(VulkanBuffer& vertexBuffer, VulkanBuffer& indexBuffer, const Data&
//     data);

//     u64 upcopyRange(
//       VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer& outBuffer,
//       u64 size, const void* data
//     );

//     VKContext& m_context;
//     VKLogicalDevice& m_device;
// };

// }  // namespace sl::vk
