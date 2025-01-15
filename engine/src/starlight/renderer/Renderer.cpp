#include "Renderer.hh"

namespace sl {

Renderer::Renderer(Context& context) :
    m_context(context), m_window(context.getWindow()),
    m_device(Device::create(m_context)),
    m_swapchain(m_device->createSwapchain(m_window.getFramebufferSize())),
    m_currentFrame(0u), m_maxFramesInFlight(m_swapchain->getImageCount()) {
    createSyncPrimitives();
}

Context& Renderer::getContext() { return m_context; }

Window& Renderer::getWindow() { return m_window; }

Device& Renderer::getDevice() { return *m_device; }

Swapchain& Renderer::getSwapchain() { return *m_swapchain; }

void Renderer::createSyncPrimitives() {
    for (u8 i = 0; i < m_maxFramesInFlight; ++i) {
        m_frameFences.push_back(m_device->createFence(Fence::State::signaled));
        m_imageAvailableSemaphores.push_back(m_device->createSemaphore());
        m_queueCompleteSemaphores.push_back(m_device->createSemaphore());
    }
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

    return imageIndex;
}

void Renderer::endFrame(u32 imageIndex) {
    auto& commandBuffer = *m_commandBuffers[imageIndex];
    commandBuffer.end();

    Queue::SubmitInfo submitInfo{
        .commandBuffer   = commandBuffer,
        .waitSemaphore   = m_imageAvailableSemaphores[imageIndex].get(),
        .signalSemaphore = m_queueCompleteSemaphores[imageIndex].get()
    };

    if (not m_device->getGraphicsQueue().submit(submitInfo)) {
    }

    Queue::PresentInfo presentInfo{
        .swapchain     = *m_swapchain,
        .imageIndex    = imageIndex,
        .waitSemaphore = m_queueCompleteSemaphores[m_currentFrame].get()
    };

    if (not m_device->getPresentQueue().present(presentInfo)) {
    }

    m_currentFrame = (m_currentFrame + 1) % m_maxFramesInFlight;
}

}  // namespace sl
