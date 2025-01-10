#pragma once

#include <vector>
#include <functional>
#include <span>

#include "starlight/core/Context.hh"

#include "RenderPacket.hh"
#include "RenderMode.hh"
#include "FrameStatistics.hh"
#include "Skybox.hh"
#include "RenderGraph.hh"

#include "camera/Camera.hh"
#include "views/RenderView.hh"
#include "gpu/RendererBackend.hh"
#include "gpu/Vendor.hh"
#include "gpu/Shader.hh"
#include "gpu/Texture.hh"

namespace sl {

class RendererFrontend : public NonCopyable {
public:
    explicit RendererFrontend(
      Window& window, EventProxy& eventProxy, const Config& config
    );

    FrameStatistics getFrameStatistics();
    RendererBackend& getRendererBackend();

    void renderFrame(
      float deltaTime, RenderPacket& packet, RenderGraph& renderGraph
    );

    void setRenderMode(RenderMode mode);

private:
    void onViewportResize(const Vec2<u32>& viewportSize);

    Window& m_window;
    EventHandlerSentinel m_eventSentinel;

    RendererBackendVendor m_backend;

    RenderMode m_renderMode;
    u16 m_framesSinceResize;
    bool m_resizing;

    FrameStatistics m_frameStatistics;
    Vec2<u32> m_viewportSize;

    // singletons, we want to expicitly specify order of initialization
    ShaderFactory m_shaderFactory;
    TextureFactory m_textureFactory;
    MaterialFactory m_materialFactory;
    MeshFactory m_meshFactory;
    SkyboxFactory m_skyboxFactory;
};

}  // namespace sl
