#include "Renderer.hh"

namespace sl {

Renderer::Renderer(Context& context) :
    m_context(context), m_window(context.getWindow()),
    m_device(Device::create(m_context)), m_currentFrame(0u),
    m_maxFramesInFlight(m_swapchain->getImageCount()) {}

Context& Renderer::getContext() { return m_context; }

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

void Renderer::endFrame(u8 imageIndex) {
    auto& commandBuffer = *m_commandBuffers[imageIndex];
    commandBuffer.end();

    auto& graphicsQueue = m_device->getQueue(Queue::Type::graphics);
    graphicsQueue.submit(
      commandBuffer, m_imageAvailableSemaphores[imageIndex].get(),
      m_queueCompleteSemaphores[imageIndex].get()
    );

    m_currentFrame = (m_currentFrame + 1) % m_maxFramesInFlight;
}

}  // namespace sl
