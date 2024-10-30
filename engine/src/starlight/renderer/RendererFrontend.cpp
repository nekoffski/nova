#include "RendererFrontend.hh"

#include "starlight/core/math/Core.hh"
#include "starlight/core/memory/Memory.hh"

#include "RenderPacket.hh"
#include "camera/EulerCamera.hh"

namespace sl {

RendererFrontend::RendererFrontend(Context& context) :
    m_backend(context.getWindow(), context.getConfig()),
    m_renderMode(RenderMode::standard), m_framesSinceResize(0u), m_resizing(false),
    m_viewportSize(context.getWindow().getFramebufferSize()),
    m_shaderManager(m_backend), m_textureManager(m_backend),
    m_meshManager(m_backend) {}

FrameStatistics RendererFrontend::getFrameStatistics() { return m_frameStatistics; }

RendererBackend& RendererFrontend::getRendererBackend() { return m_backend; }

void RendererFrontend::renderFrame(
  float deltaTime, const RenderPacket& packet, RenderGraph& renderGraph
) {
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
    }

    m_frameStatistics.renderedVertices = m_backend.renderFrame(deltaTime, [&]() {
        RenderProperties renderProperties{
            m_renderMode, m_frameStatistics.frameNumber
        };
        for (auto& view : renderGraph.getViews())
            view->render(m_backend, packet, renderProperties, deltaTime);
    });
}

void RendererFrontend::setRenderMode(RenderMode mode) {
    LOG_TRACE("Render mode set to: {}", mode);
    m_renderMode = mode;
}

void RendererFrontend::onViewportResize([[maybe_unused]] Vec2<u32> viewportSize) {
    // m_resizing = true;
    // m_backend.onViewportResize(viewportSize);

    // auto backendProxy = m_backend.getProxy();
    // for (auto& view : m_renderViews)
    //     view->onViewportResize(*backendProxy, viewportSize);
}

}  // namespace sl
