#include "GPUMemoryProxy.h"

#include "RendererBackend.h"
#include "starlight/renderer/Material.h"
#include "starlight/renderer/Geometry.h"

namespace sl {

GPUMemoryProxy::GPUMemoryProxy(RendererBackend& rendererBackend)
    : m_rendererBackend(rendererBackend) {}

void GPUMemoryProxy::acquireMaterialResources(Material& material) {
    m_rendererBackend.acquireMaterialResources(material);
}

void GPUMemoryProxy::releaseMaterialResources(Material& material) {
    m_rendererBackend.releaseMaterialResources(material);
}

void GPUMemoryProxy::acquireGeometryResources(
    Geometry& geometry, std::span<Vertex3> vertices, std::span<uint32_t> indices
) {
    m_rendererBackend.acquireGeometryResources(geometry, vertices, indices);
}

void GPUMemoryProxy::releaseGeometryResources(Geometry& geometry) {
    m_rendererBackend.releaseGeometryResources(geometry);
}

}  // namespace sl