#include "RendererFrontend.hh"

#include "starlight/core/math/Core.hh"
#include "starlight/core/event/WindowResized.hh"
#include "starlight/core/memory/Memory.hh"
#include "starlight/core/event/EventProxy.hh"
#include "starlight/core/TaskQueue.hh"

#include "RenderPacket.hh"
#include "camera/EulerCamera.hh"

namespace sl {

RendererFrontend::RendererFrontend(
  Window& window, EventProxy& eventProxy, const Config& config
) :
    m_window(window), m_eventSentinel(eventProxy), m_backend(m_window, config),
    m_renderMode(RenderMode::standard), m_framesSinceResize(0u), m_resizing(false),
    m_viewportSize(m_window.getFramebufferSize()),
    m_shaderFactory(config.paths.shaders, m_backend),
    m_textureFactory(config.paths.textures, m_backend),
    m_materialFactory(config.paths.materials), m_meshFactory(m_backend) {
    m_eventSentinel.add<WindowResized>([&](const auto& event) {
        onViewportResize(event.size);
    });
}

FrameStatistics RendererFrontend::getFrameStatistics() { return m_frameStatistics; }

RendererBackend& RendererFrontend::getRendererBackend() { return m_backend; }

void RendererFrontend::renderFrame(
  float deltaTime, RenderPacket& packet, RenderGraph& renderGraph
) {
    sl::TaskQueue::get().dispatchQueue(sl::TaskQueue::Type::preFrameRender);
    m_frameStatistics.frameNumber++;
    m_frameStatistics.deltaTime = deltaTime;

    if (m_resizing) [[unlikely]] {
        static constexpr u16 requiredFramesSinceResize = 30u;

        LOG_TRACE(
          "Resizing viewport, frame dropped {}/{}", m_framesSinceResize,
          requiredFramesSinceResize
        );

        if (m_framesSinceResize++ >= requiredFramesSinceResize) {
            m_resizing          = false;
            m_framesSinceResize = 0;
        } else {
            m_frameStatistics.renderedVertices = 0;
        }
        return;
    }

    m_frameStatistics.renderedVertices = m_backend.renderFrame(
      deltaTime,
      [&](CommandBuffer& commandBuffer, u8 imageIndex) {
          RenderProperties renderProperties{
              m_renderMode, m_frameStatistics.frameNumber
          };

          renderGraph.traverse(
            [&](auto& view, auto& renderPass) {
                renderPass.run(
                  commandBuffer, imageIndex,
                  [&](CommandBuffer& commandBuffer, u8 imageIndex) {
                      m_backend.setViewport(view.getViewport());
                      view.render(
                        m_backend, packet, renderProperties, deltaTime,
                        commandBuffer, imageIndex
                      );
                  }
                );
            },
            RenderGraph::TraverseMode::activeOnly
          );
      }
    );
    sl::TaskQueue::get().dispatchQueue(sl::TaskQueue::Type::postFrameRender);
}

void RendererFrontend::setRenderMode(RenderMode mode) {
    LOG_TRACE("Render mode set to: {}", mode);
    m_renderMode = mode;
}

void RendererFrontend::onViewportResize(const Vec2<u32>& viewportSize) {
    m_resizing = true;
    m_backend.onViewportResize(viewportSize);
}

}  // namespace sl
