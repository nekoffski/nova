#include "MeshFactory.hh"

namespace sl {

MeshFactory::MeshFactory(Buffer& vertexBuffer, Buffer& indexBuffer) :
    m_vertexBuffer(vertexBuffer), m_indexBuffer(indexBuffer) {
    createDefaults();
}

SharedPtr<Mesh> MeshFactory::create(
  const std::string& name, const Mesh::Properties2D& config
) {
    return save(name, createMesh(config.toMeshData()));
}

SharedPtr<Mesh> MeshFactory::create(
  const std::string& name, const Mesh::Properties3D& config
) {
    return save(name, createMesh(config.toMeshData()));
}

SharedPtr<Mesh> MeshFactory::getCube() { return m_cube; }
SharedPtr<Mesh> MeshFactory::getUnitSphere() { return m_unitSphere; }
SharedPtr<Mesh> MeshFactory::getPlane() { return m_plane; }

void MeshFactory::createDefaults() {
    Mesh::Properties3D unitSphere{
        SphereProperties{ 16, 16, 1.0f }
    };
    m_unitSphere = save("UnitSphere", createMesh(unitSphere.toMeshData()));

    Mesh::Properties3D plane{
        PlaneProperties{ 5.0f, 5.0f, 2, 2 }
    };
    m_plane = save("Plane", createMesh(plane.toMeshData()));

    Mesh::Properties3D cube{
        CubeProperties{ 1.0f, 1.0f, 1.0f, 1, 1 }
    };
    m_cube = save("Cube", createMesh(cube.toMeshData()));
}

SharedPtr<Mesh> MeshFactory::createMesh(const Mesh::Data& meshData) {
    return SharedPtr<Mesh>::create(meshData, m_vertexBuffer, m_indexBuffer);
}

}  // namespace sl
