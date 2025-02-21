// #include "MeshFactory.hh"

// namespace sl {

// MeshFactory::MeshFactory(Buffer& vertexBuffer, Buffer& indexBuffer) :
//     ResourceFactory("Mesh"), m_vertexBuffer(vertexBuffer),
//     m_indexBuffer(indexBuffer) {
//     createDefaults();
// }

// ResourceRef<Mesh> MeshFactory::create(
//   const std::string& name, const Mesh::Properties2D& config
// ) {
//     if (auto resource = find(name); resource) return resource;
//     return store(name, createMesh(config.toMeshData()));
// }

// ResourceRef<Mesh> MeshFactory::create(
//   const std::string& name, const Mesh::Properties3D& config
// ) {
//     if (auto resource = find(name); resource) return resource;
//     return store(name, createMesh(config.toMeshData()));
// }

// ResourceRef<Mesh> MeshFactory::getCube() { return m_cube; }

// ResourceRef<Mesh> MeshFactory::getPlane() { return m_plane; }

// ResourceRef<Mesh> MeshFactory::getUnitSphere() { return m_unitSphere; }

// void MeshFactory::createDefaults() {
//     Mesh::Properties3D unitSphereConfig{
//         SphereProperties{ 16, 16, 1.0f }
//     };
//     m_unitSphere = create("UnitSphere", unitSphereConfig);

//     Mesh::Properties3D planeConfig{
//         PlaneProperties{ 5.0f, 5.0f, 2, 2 }
//     };
//     m_plane = create("Plane", planeConfig);

//     Mesh::Properties3D cubeConfig{
//         CubeProperties{ 1.0f, 1.0f, 1.0f, 1, 1 }
//     };
//     m_cube = create("Cube", cubeConfig);
// }

// UniquePointer<Mesh> MeshFactory::createMesh(const Mesh::Data& meshData) {
//     return UniquePointer<Mesh>::create(meshData, m_vertexBuffer, m_indexBuffer);
// }

// }  // namespace sl
