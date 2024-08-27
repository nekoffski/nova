#pragma once

#include "fwd.hh"

#include "starlight/renderer/gpu/RendererBackendProxy.hh"
#include "VKCommandBuffer.hh"

namespace sl::vk {

class VKRendererBackendProxy : public RendererBackendProxy {
public:
    explicit VKRendererBackendProxy(VKRendererBackend* backend);

    void drawMesh(const Mesh& mesh) override;
    VKCommandBuffer* getCommandBuffer() override;
    u32 getImageIndex() override;

    Texture* getFramebuffer(u64 id) override;
    Texture* getDepthBuffer() override;

    void setViewport(const Rect2<u32>& viewport) override;

    void gpuCall(std::function<void(CommandBuffer&)>&&) override;

    UniqPtr<UIRenderer> createUIRendererer(RenderPass* renderPass) override;

private:
    VKRendererBackend* m_backend;
};
}  // namespace sl::vk