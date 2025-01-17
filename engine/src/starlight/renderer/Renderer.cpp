#include "Renderer.hh"

#include "starlight/core/math/Vertex.hh"

namespace sl {

static constexpr u64 bufferSize = 1024 * 1024;

static OwningPtr<Buffer> createVertexBuffer(Device& device) {
    return device.createBuffer(Buffer::Properties{
      .size           = bufferSize * sizeof(Vertex3),
      .memoryProperty = MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      .usage =
        BufferUsage::BUFFER_USAGE_TRANSFER_DST_BIT
        | BufferUsage::BUFFER_USAGE_TRANSFER_SRC_BIT
        | BufferUsage::BUFFER_USAGE_VERTEX_BUFFER_BIT,
      .bindOnCreate = true,
    });
}

static OwningPtr<Buffer> createIndexBuffer(Device& device) {
    return device.createBuffer(Buffer::Properties{
      .size           = bufferSize * sizeof(u32),
      .memoryProperty = MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      .usage =
        BufferUsage::BUFFER_USAGE_TRANSFER_DST_BIT
        | BufferUsage::BUFFER_USAGE_TRANSFER_SRC_BIT
        | BufferUsage::BUFFER_USAGE_INDEX_BUFFER_BIT,
      .bindOnCreate = true,
    });
}

Renderer::Renderer(Context& context) :
    m_context(context), m_window(context.getWindow()), m_config(context.getConfig()),
    m_device(Device::create(m_context)),
    m_swapchain(m_device->createSwapchain(m_window.getFramebufferSize())),
    m_vertexBuffer(createVertexBuffer(*m_device)),
    m_indexBuffer(createIndexBuffer(*m_device)), m_currentFrame(0u),
    m_maxFramesInFlight(m_swapchain->getImageCount()),
    m_shaderFactory(m_config.paths.shaders, *m_device),
    m_textureFactory(m_config.paths.textures, *m_device),
    m_materialFactory(m_config.paths.materials),
    m_meshFactory(*m_vertexBuffer, *m_indexBuffer) {
    createSyncPrimitives();
    createBuffers();
}

Context& Renderer::getContext() { return m_context; }

Window& Renderer::getWindow() { return m_window; }

Device& Renderer::getDevice() { return *m_device; }

Swapchain& Renderer::getSwapchain() { return *m_swapchain; }

void Renderer::createSyncPrimitives() {
    for (u8 i = 0; i < m_maxFramesInFlight; ++i) {
        m_frameFences.push_back(m_device->createFence(Fence::State::signaled));
        m_imageFences.push_back(nullptr);
        m_imageAvailableSemaphores.push_back(m_device->createSemaphore());
        m_queueCompleteSemaphores.push_back(m_device->createSemaphore());
    }
}

void Renderer::createBuffers() {
    for (u8 i = 0; i < m_maxFramesInFlight; ++i)
        m_commandBuffers.push_back(m_device->createCommandBuffer());
}

std::optional<u8> Renderer::beginFrame() {
    m_frameFences[m_currentFrame]->wait();

    auto imageIndex = m_swapchain->acquireNextImageIndex(
      m_imageAvailableSemaphores[m_currentFrame].get()
    );
    if (not imageIndex) return {};

    auto& commandBuffer = *m_commandBuffers[*imageIndex];
    commandBuffer.begin();

    const auto& framebufferSize = m_window.getFramebufferSize();

    commandBuffer.execute(SetViewportCommand{
      .offset = Vec2<u32>{ 0u, 0u },
      .size   = framebufferSize,
    });

    commandBuffer.execute(SetScissorsCommand{
      .offset = Vec2<u32>{ 0u, 0u },
      .size   = framebufferSize,
    });

    commandBuffer.execute(BindVertexBufferCommand{
      .buffer = m_vertexBuffer.get(),
      .offset = 0u,
    });

    commandBuffer.execute(BindIndexBufferCommand{
      .buffer = m_indexBuffer.get(),
      .offset = 0u,
    });

    return imageIndex;
}

Fence* Renderer::getImageFence(u32 imageIndex) {
    if (auto& fence = m_imageFences[imageIndex]; fence) fence->wait();

    auto frameFence = m_frameFences[m_currentFrame].get();
    frameFence->reset();
    m_imageFences[imageIndex] = frameFence;

    return frameFence;
}

void Renderer::endFrame(u32 imageIndex) {
    auto& commandBuffer = *m_commandBuffers[imageIndex];
    commandBuffer.end();

    Queue::SubmitInfo submitInfo{
        .commandBuffer   = commandBuffer,
        .waitSemaphore   = m_imageAvailableSemaphores[imageIndex].get(),
        .signalSemaphore = m_queueCompleteSemaphores[imageIndex].get(),
        .fence           = getImageFence(imageIndex)
    };

    if (not m_device->getGraphicsQueue().submit(submitInfo)) {
    }

    Queue::PresentInfo presentInfo{
        .swapchain     = *m_swapchain,
        .imageIndex    = imageIndex,
        .waitSemaphore = m_queueCompleteSemaphores[m_currentFrame].get(),
    };

    if (not m_device->getPresentQueue().present(presentInfo)) {
    }

    m_currentFrame = (m_currentFrame + 1) % m_maxFramesInFlight;
}

}  // namespace sl
