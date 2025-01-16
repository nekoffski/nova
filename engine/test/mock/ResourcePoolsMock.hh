#include "starlight/renderer/gpu/ResourcePools.hh"

#include <gmock/gmock.h>

using namespace sl;

struct ResourcePoolsMock : public ResourcePools {
    MOCK_METHOD(
      Mesh*, createMesh,
      (const Mesh::Properties&, std::span<const Vertex3>, std::span<const uint32_t>,
       const Extent3&)
    );
    MOCK_METHOD(
      Mesh*, createMesh,
      (const Mesh::Properties&, std::span<const Vertex2>, std::span<const uint32_t>,
       const Extent2&)
    );
    MOCK_METHOD(void, destroyMesh, (Mesh&));
    MOCK_METHOD(
      Texture*, createTexture, (const Texture::Properties&, const std::span<u8>)
    );
    MOCK_METHOD(void, destroyTexture, (Texture&));
    MOCK_METHOD(Shader*, createShader, (const Shader::Properties&));
    MOCK_METHOD(void, destroyShader, (Shader&));
    MOCK_METHOD(
      RenderTarget*, createRenderTarget,
      (const RenderTarget::Properties&, RenderPass*)
    );
    MOCK_METHOD(void, destroyRenderTarget, (RenderTarget&));
    MOCK_METHOD(
      RenderPass*, createRenderPass, (const RenderPassBackend::Properties&)
    );
    MOCK_METHOD(void, destroyRenderPass, (RenderPass&));
};
