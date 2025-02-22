#pragma once

#include "starlight/core/Factory.hh"
#include "starlight/renderer/Mesh.hh"

namespace sl {

class MeshFactory : public Factory<MeshFactory, Mesh> {
public:
    explicit MeshFactory(Buffer& vertexBuffer, Buffer& indexBuffer);

    template <typename T>
    requires std::is_constructible_v<Mesh::Properties3D, const T&>
    SharedPointer<Mesh> create(const std::string& name, const T& properties) {
        return create(name, Mesh::Properties3D{ properties });
    }

    SharedPointer<Mesh> create(
      const std::string& name, const Mesh::Properties2D& config
    );
    SharedPointer<Mesh> create(
      const std::string& name, const Mesh::Properties3D& config
    );

    SharedPointer<Mesh> getCube();
    SharedPointer<Mesh> getUnitSphere();
    SharedPointer<Mesh> getPlane();

private:
    SharedPointer<Mesh> createMesh(const Mesh::Data& meshData);

    void createDefaults();

    Buffer& m_vertexBuffer;
    Buffer& m_indexBuffer;

    SharedPointer<Mesh> m_unitSphere;
    SharedPointer<Mesh> m_plane;
    SharedPointer<Mesh> m_cube;
};

}  // namespace sl
