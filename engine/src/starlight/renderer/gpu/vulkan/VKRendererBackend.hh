// #pragma once

// #include <array>
// #include <cstdint>
// #include <span>
// #include <vector>

// #include "starlight/core/Core.hh"
// #include "starlight/core/Config.hh"
// #include "starlight/core/fwd.hh"
// #include "starlight/core/memory/Memory.hh"
// #include "starlight/core/window/Window.hh"
// #include "starlight/renderer/gpu/RendererBackend.hh"

// #include "Vulkan.hh"
// #include "fwd.hh"

// #include "starlight/renderer/gpu/Shader.hh"

// #include "VKMesh.hh"
// #include "VKPipeline.hh"
// #include "VKSemaphore.hh"
// #include "VKPhysicalDevice.hh"
// #include "VKLogicalDevice.hh"
// #include "VKTexture.hh"
// #include "VulkanBuffer.hh"
// #include "VKUIRenderer.hh"

// namespace sl::vk {

// class VKRendererBackend : public RendererBackend {
// public:
//     explicit VKRendererBackend(sl::Window& window, const Config& config);
//     ~VKRendererBackend();

//     bool beginFrame(float deltaTime) override;
//     bool endFrame(float deltaTime) override;

//     void drawMesh(const Mesh& mesh) override;
//     void onViewportResize(const Vec2<u32>& viewportSize) override;

//     u64 getRenderedVertexCount() const override;
//     void setViewport(const Rect2<u32>& viewport);

//     VulkanBuffer& getIndexBuffer();
//     VulkanBuffer& getVertexBuffer();
//     VulkanSwapchain& getSwapchain();
//     VKContext& getContext();
//     VKLogicalDevice& getLogicalDevice();
//     VKPhysicalDevice& getPhysicalDevice();

//     void setViewport(VulkanCommandBuffer& commandBuffer, const Rect2<u32>&
//     viewport); void setScissors(VulkanCommandBuffer& commandBuffer);

//     VulkanCommandBuffer& getCommandBuffer() override;
//     u32 getImageIndex() const override;
//     u32 getSwapchainImageCount() const override;

//     VKTexture* getSwapchainTexture(u32 index) override;
//     VKTexture* getDepthTexture() override;

//     Window& getWindow();

// private:
//     void gpuCall(std::function<void(CommandBuffer&)>&& callback);

//     void createCoreComponents(sl::Window& window);
//     void createCommandBuffers();
//     void createSemaphoresAndFences();
//     void freeDataRange(VulkanBuffer& buffer, uint64_t offset, uint64_t size);
//     void createBuffers();
//     void recreateSwapchain();

//     VKFence* acquireImageFence();

//     bool m_recreatingSwapchain;

//     Window& m_window;

//     VKContext m_context;
//     VKPhysicalDevice m_physicalDevice;
//     VKLogicalDevice m_logicalDevice;

//     UniqPtr<VulkanSwapchain> m_swapchain;

//     // vulkan

//     // TODO: consider creating as ptrs to allow mocking
//     UniqPtr<VulkanBuffer> m_objectVertexBuffer;
//     UniqPtr<VulkanBuffer> m_objectIndexBuffer;

//     VkDescriptorPool m_uiPool;

//     u64 m_renderedVertices;
//     u32 m_maxFramesInFlight;

//     u32 m_imageIndex   = 0;
//     u32 m_currentFrame = 0;

//     u32 m_framebufferWidth;
//     u32 m_framebufferHeight;

//     std::vector<LocalPtr<VulkanCommandBuffer>> m_commandBuffers;
//     std::vector<LocalPtr<VKSemaphore>> m_imageAvailableSemaphores;
//     std::vector<LocalPtr<VKSemaphore>> m_queueCompleteSemaphores;
//     std::vector<LocalPtr<VKFence>> m_inFlightFences;
//     std::vector<VKFence*> m_imagesInFlight;
// };

// }  // namespace sl::vk
