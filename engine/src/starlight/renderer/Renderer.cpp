#include "Renderer.hh"

#include "starlight/core/math/Vertex.hh"
#include "starlight/window/Window.hh"

#include "starlight/window/Events.hh"
#include "starlight/core/Globals.hh"

namespace sl {

static constexpr u64 bufferSize = 1024 * 1024;

static UniquePointer<Buffer> createVertexBuffer();
static UniquePointer<Buffer> createIndexBuffer();

Renderer::Renderer() :
    m_swapchain(Swapchain::create()), m_vertexBuffer(createVertexBuffer()),
    m_indexBuffer(createIndexBuffer()), m_currentFrame(0u),
    m_maxFramesInFlight(m_swapchain->getImageCount()), m_frameNumber(0u),
    m_shaderFactory(Globals::get().getConfig().paths.shaders, Device::get()),
    m_textureFactory(Globals::get().getConfig().paths.textures, Device::get()),
    m_materialFactory(Globals::get().getConfig().paths.materials),
    m_meshFactory(*m_vertexBuffer, *m_indexBuffer),
    m_eventSentinel(EventProxy::get()), m_recreatingSwapchain(false),
    m_framesSinceResize(0u) {
    createSyncPrimitives();
    createBuffers();
    initEventHandlers();
}

Swapchain& Renderer::getSwapchain() { return *m_swapchain; }

Buffer& Renderer::getIndexBuffer() { return *m_indexBuffer; }

Buffer& Renderer::getVertexBuffer() { return *m_vertexBuffer; }

void Renderer::createSyncPrimitives() {
    m_frameFences.clear();
    m_imageFences.clear();
    m_imageAvailableSemaphores.clear();
    m_queueCompleteSemaphores.clear();

    for (u8 i = 0; i < m_maxFramesInFlight; ++i) {
        m_frameFences.push_back(Fence::create(Fence::State::signaled));
        m_imageFences.push_back(nullptr);
        m_imageAvailableSemaphores.push_back(Semaphore::create());
        m_queueCompleteSemaphores.push_back(Semaphore::create());
    }
}

void Renderer::createBuffers() {
    for (u8 i = 0; i < m_maxFramesInFlight; ++i)
        m_commandBuffers.push_back(CommandBuffer::create());
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

    Device::get().waitIdle();
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

    const auto& framebufferSize = Window::get().getFramebufferSize();

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

    auto& device = Device::get();

    if (device.getGraphicsQueue().submit(submitInfo)) [[likely]] {
        Queue::PresentInfo presentInfo{
            .swapchain     = *m_swapchain,
            .imageIndex    = imageIndex,
            .waitSemaphore = m_queueCompleteSemaphores[m_currentFrame].get(),
        };
        if (not device.getPresentQueue().present(presentInfo)) [[unlikely]]
            log::warn("Could not present image");
    } else {
        log::warn("Could not submit graphics queue");
    }

    m_currentFrame = (m_currentFrame + 1) % m_maxFramesInFlight;
}

UniquePointer<Buffer> createVertexBuffer() {
    return Buffer::create(Buffer::Properties{
      .size           = bufferSize * sizeof(Vertex3),
      .memoryProperty = MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      .usage =
        BufferUsage::BUFFER_USAGE_TRANSFER_DST_BIT
        | BufferUsage::BUFFER_USAGE_TRANSFER_SRC_BIT
        | BufferUsage::BUFFER_USAGE_VERTEX_BUFFER_BIT,
      .bindOnCreate = true,
    });
}

UniquePointer<Buffer> createIndexBuffer() {
    return Buffer::create(Buffer::Properties{
      .size           = bufferSize * sizeof(u32),
      .memoryProperty = MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      .usage =
        BufferUsage::BUFFER_USAGE_TRANSFER_DST_BIT
        | BufferUsage::BUFFER_USAGE_TRANSFER_SRC_BIT
        | BufferUsage::BUFFER_USAGE_INDEX_BUFFER_BIT,
      .bindOnCreate = true,
    });
}

}  // namespace sl
