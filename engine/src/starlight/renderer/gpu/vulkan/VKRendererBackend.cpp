// #include "VKRendererBackend.hh"

// #include "VulkanBuffer.hh"
// #include "VulkanCommandBuffer.hh"
// #include "VKContext.hh"
// #include "VKFence.hh"
// #include "VKFramebuffer.hh"
// #include "VKImage.hh"
// #include "VKRenderPass.hh"
// #include "VulkanSwapchain.hh"
// #include "VKShader.hh"

// namespace sl::vk {

// // TODO: load from config

// std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

// VKPhysicalDevice::Requirements physicalDeviceRequirements{
//     .supportedQueues =
//       VKPhysicalDevice::Queue::graphics | VKPhysicalDevice::Queue::present
//       | VKPhysicalDevice::Queue::transfer,
//     .isDiscreteGPU             = true,
//     .supportsSamplerAnisotropy = true,
//     .extensions                = extensions,
// };

// VKRendererBackend::VKRendererBackend(Window& window, const Config& config) :
//     m_recreatingSwapchain(false), m_window(window), m_context(window, config),
//     m_physicalDevice(m_context, physicalDeviceRequirements),
//     m_logicalDevice(m_context, m_physicalDevice), m_renderedVertices(0u) {
//     auto size = window.getFramebufferSize();

//     m_framebufferWidth  = size.w;
//     m_framebufferHeight = size.h;

//     createCoreComponents(window);
//     createSemaphoresAndFences();
//     createCommandBuffers();
// }

// VKRendererBackend::~VKRendererBackend() { m_logicalDevice.waitIdle(); }

// // TODO: remove
// void VKRendererBackend::gpuCall(std::function<void(CommandBuffer& buffer)>&&
// callback ) {
//     const auto queue = m_logicalDevice.getQueues().graphics;

//     vkQueueWaitIdle(queue);
//     VulkanCommandBuffer commandBuffer(
//       m_logicalDevice, m_logicalDevice.getGraphicsCommandPool(),
//       VulkanCommandBuffer::Severity::primary
//     );
//     commandBuffer.createAndBeginSingleUse();
//     callback(commandBuffer);
//     commandBuffer.endSingleUse(queue);
//     m_logicalDevice.waitIdle();
// }

// void VKRendererBackend::freeDataRange(
//   VulkanBuffer& buffer, uint64_t offset, uint64_t size
// ) {
//     buffer.free(size, offset);
// }

// void VKRendererBackend::drawMesh(const Mesh& mesh) {
//     const auto& dataDescription = mesh.getDataDescription();
//     auto commandBuffer          = m_commandBuffers[m_imageIndex].get();

//     VkDeviceSize offsets[1] = { dataDescription.vertexBufferOffset };

//     vkCmdBindVertexBuffers(
//       commandBuffer->getHandle(), 0, 1, m_objectVertexBuffer->getHandlePointer(),
//       (VkDeviceSize*)offsets
//     );
//     if (dataDescription.indexCount > 0) {
//         vkCmdBindIndexBuffer(
//           commandBuffer->getHandle(), m_objectIndexBuffer->getHandle(),
//           dataDescription.indexBufferOffset, VK_INDEX_TYPE_UINT32
//         );
//         vkCmdDrawIndexed(
//           commandBuffer->getHandle(), dataDescription.indexCount, 1, 0, 0, 0
//         );

//         m_renderedVertices += dataDescription.indexCount;
//     } else {
//         vkCmdDraw(commandBuffer->getHandle(), dataDescription.vertexCount, 1, 0,
//         0); m_renderedVertices += dataDescription.vertexCount;
//     }
// }

// void VKRendererBackend::createBuffers() {
//     LOG_DEBUG("Creating buffers");

//     m_objectVertexBuffer = createUniqPtr<VulkanBuffer>(
//       m_context, m_logicalDevice,
//       VulkanBuffer::Properties{
//         sizeof(Vertex3) * 1024 * 1024, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
//           | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//         true }
//     );

//     m_objectIndexBuffer = createUniqPtr<VulkanBuffer>(
//       m_context, m_logicalDevice,
//       VulkanBuffer::Properties{
//         sizeof(u32) * 1024 * 1024, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//         VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
//           | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//         true }
//     );
// }

// VulkanCommandBuffer& VKRendererBackend::getCommandBuffer() {
//     return *m_commandBuffers[m_imageIndex];
// }

// u32 VKRendererBackend::getImageIndex() const { return m_imageIndex; }

// u32 VKRendererBackend::getSwapchainImageCount() const {
//     return m_swapchain->getImageCount();
// }

// VKTexture* VKRendererBackend::getSwapchainTexture(u32 index) {
//     return m_swapchain->getFramebuffer(index);
// }

// VKTexture* VKRendererBackend::getDepthTexture() {
//     return m_swapchain->getDepthBuffer();
// }

// Window& VKRendererBackend::getWindow() { return m_window; }

// void VKRendererBackend::createCoreComponents(sl::Window& window) {
//     m_swapchain = createUniqPtr<VulkanSwapchain>(
//       m_context, m_logicalDevice, window.getFramebufferWidth(),
//       window.getFramebufferHeight()
//     );
//     m_maxFramesInFlight = m_swapchain->getImageCount();

//     createBuffers();
// }

// void VKRendererBackend::createSemaphoresAndFences() {
//     m_imageAvailableSemaphores.reserve(m_maxFramesInFlight);
//     m_queueCompleteSemaphores.reserve(m_maxFramesInFlight);

//     m_imagesInFlight.resize(m_maxFramesInFlight);
//     m_inFlightFences.reserve(m_maxFramesInFlight);

//     for (u32 i = 0; i < m_maxFramesInFlight; ++i) {
//         m_imageAvailableSemaphores.emplace_back(m_context, m_logicalDevice);
//         m_queueCompleteSemaphores.emplace_back(m_context, m_logicalDevice);
//         m_inFlightFences.emplace_back(
//           m_context, m_logicalDevice, VKFence::State::signaled
//         );
//     }
// }

// void VKRendererBackend::onViewportResize(const Vec2<u32>& viewportSize) {
//     m_framebufferWidth    = viewportSize.w;
//     m_framebufferHeight   = viewportSize.h;
//     m_recreatingSwapchain = true;
//     LOG_TRACE(
//       "Vulkan renderer backend framebuffer resized {}/{}", m_framebufferWidth,
//       m_framebufferHeight
//     );

//     m_logicalDevice.waitIdle();
//     m_inFlightFences[m_currentFrame]->wait(UINT64_MAX);
//     recreateSwapchain();

//     m_recreatingSwapchain = false;
// }

// u64 VKRendererBackend::getRenderedVertexCount() const { return m_renderedVertices;
// }

// void VKRendererBackend::setViewport(const Rect2<u32>& viewport) {
//     setViewport(*m_commandBuffers[m_imageIndex], viewport);
// }

// void VKRendererBackend::createCommandBuffers() {
//     const auto graphicsCommandPool = m_logicalDevice.getGraphicsCommandPool();

//     const auto swapchainImagesCount = m_swapchain->getImageCount();
//     m_commandBuffers.clear();
//     LOG_TRACE("Creating {} command buffers", swapchainImagesCount);
//     m_commandBuffers.reserve(swapchainImagesCount);
//     for (u32 i = 0; i < swapchainImagesCount; ++i) {
//         m_commandBuffers.emplace_back(
//           m_logicalDevice, graphicsCommandPool,
//           VulkanCommandBuffer::Severity::primary
//         );
//     }
// }

// void VKRendererBackend::recreateSwapchain() {
//     m_logicalDevice.waitIdle();

//     // TODO: implement case when recreation fails
//     m_swapchain->recreate(m_framebufferWidth, m_framebufferHeight);
//     createCommandBuffers();
//     LOG_INFO("Resized, booting.");
// }

// VKFence* VKRendererBackend::acquireImageFence() {
//     auto& fence = m_imagesInFlight[m_imageIndex];
//     if (fence) fence->wait(UINT64_MAX);
//     fence = m_inFlightFences[m_currentFrame].get();
//     fence->reset();

//     return fence;
// }

// VulkanBuffer& VKRendererBackend::getIndexBuffer() { return *m_objectIndexBuffer; }

// VulkanBuffer& VKRendererBackend::getVertexBuffer() { return *m_objectVertexBuffer;
// }

// VulkanSwapchain& VKRendererBackend::getSwapchain() { return *m_swapchain; }

// VKContext& VKRendererBackend::getContext() { return m_context; }

// VKLogicalDevice& VKRendererBackend::getLogicalDevice() { return m_logicalDevice; }

// VKPhysicalDevice& VKRendererBackend::getPhysicalDevice() { return
// m_physicalDevice; }

// void VKRendererBackend::setViewport(
//   VulkanCommandBuffer& commandBuffer, const Rect2<u32>& viewport
// ) {
//     VkViewport vkViewport;
//     vkViewport.x        = static_cast<float>(viewport.offset.x);
//     vkViewport.y        = static_cast<float>(viewport.size.h);
//     vkViewport.width    = static_cast<float>(viewport.size.w);
//     vkViewport.height   = -static_cast<float>(viewport.size.h);
//     vkViewport.minDepth = 0.0f;
//     vkViewport.maxDepth = 1.0f;

//     vkCmdSetViewport(commandBuffer.getHandle(), 0, 1, &vkViewport);
// }

// void VKRendererBackend::setScissors(VulkanCommandBuffer& commandBuffer) {
//     VkRect2D scissor;
//     scissor.offset.x = scissor.offset.y = 0;
//     scissor.extent.width                = m_framebufferWidth;
//     scissor.extent.height               = m_framebufferHeight;

//     vkCmdSetScissor(commandBuffer.getHandle(), 0, 1, &scissor);
// }

// bool VKRendererBackend::beginFrame([[maybe_unused]] float deltaTime) {
//     m_renderedVertices       = 0u;
//     const auto logicalDevice = m_logicalDevice.getHandle();

//     if (m_recreatingSwapchain) {
//         m_recreatingSwapchain = false;
//         if (auto result = vkDeviceWaitIdle(logicalDevice); not isGood(result)) {
//             LOG_ERROR("vkDeviceWaitDile failed: %s", getResultString(result,
//             true)); return false;
//         }
//         return false;
//     }

//     // Check if the framebuffer has been resized. If so, a new swapchain must be
//     // created.

//     // Wait for the execution of the current frame to complete. The fence being
//     // free will allow this one to move on.
//     if (not m_inFlightFences[m_currentFrame]->wait(UINT64_MAX)) {
//         LOG_WARN("In-flight fence wait failure!");
//         return false;
//     }

//     // Acquire the next image from the swap chain. Pass along the semaphore that
//     // should signaled when this completes. This same semaphore will later be
//     // waited on by the queue submission to ensure this image is available.
//     auto nextImageIndex = m_swapchain->acquireNextImageIndex(
//       UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame]->getHandle(), nullptr
//     );

//     if (not nextImageIndex) return false;

//     m_imageIndex = *nextImageIndex;

//     // Begin recording commands.
//     auto commandBuffer = m_commandBuffers[m_imageIndex].get();

//     commandBuffer->reset();
//     commandBuffer->begin(VulkanCommandBuffer::BeginFlags{
//       .isSingleUse          = false,
//       .isRenderpassContinue = false,
//       .isSimultaneousUse    = false,
//     });

//     setViewport(
//       *commandBuffer,
//       Rect2<u32>{
//         Vec2<u32>{ 0u,                 0u                  },
//         Vec2<u32>{ m_framebufferWidth, m_framebufferHeight },
//     }
//     );
//     setScissors(*commandBuffer);

//     return true;
// }

// bool VKRendererBackend::endFrame([[maybe_unused]] float deltaTime) {
//     auto commandBuffer = m_commandBuffers[m_imageIndex].get();

//     commandBuffer->end();

//     // Make sure the previous frame is not using this image (i.e. its fence is
//     // being waited on) if (m_context.images_in_flight[m_context.image_index] !=
//     // VK_NULL_HANDLE) {  // was frame vulkan_fence_wait(&m_context,
//     // m_context.images_in_flight[m_context.image_index], UINT64_MAX);
//     // }
//     auto fence = acquireImageFence();

//     // Submit the queue and wait for the operation to complete.
//     // Begin queue submission
//     VkSubmitInfo submit_info;
//     clearMemory(&submit_info);
//     submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

//     auto commandBufferHandle = commandBuffer->getHandle();

//     // Command buffer(s) to be executed.
//     submit_info.commandBufferCount = 1;
//     submit_info.pCommandBuffers    = &commandBufferHandle;

//     // The semaphore(s) to be signaled when the queue is complete.
//     submit_info.signalSemaphoreCount = 1;
//     submit_info.pSignalSemaphores =
//       m_queueCompleteSemaphores[m_currentFrame]->getHandlePointer();

//     // Wait semaphore ensures that the operation cannot begin until the image is
//     // available.
//     submit_info.waitSemaphoreCount = 1;
//     submit_info.pWaitSemaphores =
//       m_imageAvailableSemaphores[m_currentFrame]->getHandlePointer();

//     submit_info.pWaitDstStageMask = 0;

//     // Each semaphore waits on the corresponding pipeline stage to complete. 1:1
//     // ratio. VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent
//     // colour attachment writes from executing until the semaphore signals (i.e.
//     // one frame is presented at atime)
//     VkPipelineStageFlags flags[1] = {
//         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
//     };
//     submit_info.pWaitDstStageMask = flags;

//     const auto& deviceQueues = m_logicalDevice.getQueues();

//     VkResult result =
//       vkQueueSubmit(deviceQueues.graphics, 1, &submit_info, fence->getHandle());

//     if (result != VK_SUCCESS) {
//         LOG_ERROR(
//           "vkQueueSubmit failed with result: {}", getResultString(result, true)
//         );
//         return false;
//     }

//     commandBuffer->updateSubmitted();

//     m_swapchain->present(
//       deviceQueues.graphics, deviceQueues.present,
//       m_queueCompleteSemaphores[m_currentFrame]->getHandle(), m_imageIndex
//     );

//     m_currentFrame = (m_currentFrame + 1) % m_maxFramesInFlight;

//     return true;
// }

// }  // namespace sl::vk
