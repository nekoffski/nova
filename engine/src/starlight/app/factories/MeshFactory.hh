#pragma once

#include "starlight/core/Factory.hh"
#include "starlight/renderer/Mesh.hh"

namespace sl {

class MeshFactory : public Factory<MeshFactory, Mesh> {
public:
    explicit MeshFactory(Buffer& vertexBuffer, Buffer& indexBuffer);

    template <typename T>
    requires std::is_constructible_v<Mesh::Properties3D, const T&>
    SharedPtr<Mesh> create(const std::string& name, const T& properties) {
        return create(name, Mesh::Properties3D{ properties });
    }

    SharedPtr<Mesh> create(
      const std::string& name, const Mesh::Properties2D& config
    );
    SharedPtr<Mesh> create(
      const std::string& name, const Mesh::Properties3D& config
    );

    SharedPtr<Mesh> getCube();
    SharedPtr<Mesh> getUnitSphere();
    SharedPtr<Mesh> getPlane();

private:
    SharedPtr<Mesh> createMesh(const Mesh::Data& meshData, const std::string& name);

    void createDefaults();

    Buffer& m_vertexBuffer;
    Buffer& m_indexBuffer;

    SharedPtr<Mesh> m_unitSphere;
    SharedPtr<Mesh> m_plane;
    SharedPtr<Mesh> m_cube;
};

}  // namespace sl
