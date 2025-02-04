#include "Renderer.hh"

#include "starlight/core/math/Vertex.hh"

#include "starlight/core/event/WindowResized.hh"

namespace sl {

static constexpr u64 bufferSize = 1024 * 1024;

static UniquePointer<Buffer> createVertexBuffer(Device& device) {
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

static UniquePointer<Buffer> createIndexBuffer(Device& device) {
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
    m_maxFramesInFlight(m_swapchain->getImageCount()), m_frameNumber(0u),
    m_shaderFactory(m_config.paths.shaders, *m_device),
    m_shaderProgramFactory(m_config.paths.shaders),
    m_textureFactory(m_config.paths.textures, *m_device),
    m_materialFactory(m_config.paths.materials),
    m_meshFactory(*m_vertexBuffer, *m_indexBuffer),
    m_eventSentinel(context.getEventProxy()), m_recreatingSwapchain(false),
    m_framesSinceResize(0u) {
    createSyncPrimitives();
    createBuffers();
    initEventHandlers();
}

Context& Renderer::getContext() { return m_context; }

Window& Renderer::getWindow() { return m_window; }

Device& Renderer::getDevice() { return *m_device; }

Swapchain& Renderer::getSwapchain() { return *m_swapchain; }

Buffer& Renderer::getIndexBuffer() { return *m_indexBuffer; }

Buffer& Renderer::getVertexBuffer() { return *m_vertexBuffer; }

void Renderer::createSyncPrimitives() {
    m_frameFences.clear();
    m_imageFences.clear();
    m_imageAvailableSemaphores.clear();
    m_queueCompleteSemaphores.clear();

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

void Renderer::initEventHandlers() {
    log::trace("Setting up renderer event handlers");
    m_eventSentinel.add<WindowResized>([&](auto& event) {
        onWindowResize(event.size);
    });
}

void Renderer::onWindowResize(const Vec2<u32>& size) {
    log::debug("Window resized: {}/{} - recreating swapchain", size.w, size.h);
    m_recreatingSwapchain = true;
    m_framesSinceResize   = 0;

    m_device->waitIdle();
    m_swapchain->recreate(size);
    createSyncPrimitives();
    m_currentFrame = 0u;
}

std::optional<u8> Renderer::beginFrame() {
    if (m_recreatingSwapchain) [[unlikely]] {
        static constexpr u64 framesToDrop = 60u;
        m_recreatingSwapchain = (++m_framesSinceResize) <= framesToDrop - 1;
        log::trace(
          "Recreating swapchain, dropping frame: {}/{}", m_framesSinceResize,
          framesToDrop
        );
        return {};
    }

    m_frameFences[m_currentFrame]->wait();

    auto imageIndex = m_swapchain->acquireNextImageIndex(
      m_imageAvailableSemaphores[m_currentFrame].get()
    );
    if (not imageIndex) {
        return {};
    }

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
      .buffer = *m_vertexBuffer,
      .offset = 0u,
    });

    commandBuffer.execute(BindIndexBufferCommand{
      .buffer = *m_indexBuffer,
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

    if (not m_device->getGraphicsQueue().submit(submitInfo)) [[unlikely]] {
    }

    Queue::PresentInfo presentInfo{
        .swapchain     = *m_swapchain,
        .imageIndex    = imageIndex,
        .waitSemaphore = m_queueCompleteSemaphores[m_currentFrame].get(),
    };

    if (not m_device->getPresentQueue().present(presentInfo)) [[unlikely]] {
    }

    m_currentFrame = (m_currentFrame + 1) % m_maxFramesInFlight;
}

}  // namespace sl
