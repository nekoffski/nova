#pragma once

#include <string>
#include <span>

#include "starlight/core/math/Extent.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/core/math/Vertex.hh"
#include "starlight/core/Resource.hh"

#include "starlight/renderer/Mesh.hh"
#include "starlight/renderer/gpu/Device.hh"
#include "starlight/renderer/gpu/fwd.hh"
#include "starlight/renderer/gpu/Buffer.hh"

namespace sl {

class MeshFactory
    : public ResourceFactory<Mesh>,
      public kc::core::Singleton<MeshFactory> {
public:
    explicit MeshFactory(Buffer& vertexBuffer, Buffer& indexBuffer);

    template <typename T>
    requires std::is_constructible_v<Mesh::Properties3D, const T&>
    ResourceRef<Mesh> create(const std::string& name, const T& properties) {
        return create(name, Mesh::Properties3D{ properties });
    }

    ResourceRef<Mesh> create(
      const std::string& name, const Mesh::Properties2D& config
    );
    ResourceRef<Mesh> create(
      const std::string& name, const Mesh::Properties3D& config
    );

    ResourceRef<Mesh> getCube();
    ResourceRef<Mesh> getUnitSphere();
    ResourceRef<Mesh> getPlane();

private:
    void createDefaults();

    OwningPtr<Mesh> createMesh(const Mesh::Data& meshData);

    Buffer& m_vertexBuffer;
    Buffer& m_indexBuffer;

    ResourceRef<Mesh> m_unitSphere;
    ResourceRef<Mesh> m_plane;
    ResourceRef<Mesh> m_cube;
};

}  // namespace sl
