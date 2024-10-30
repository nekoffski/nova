#include "Mesh.hh"

#include "starlight/core/math/Geometry.hh"
#include "starlight/core/math/Vertex.hh"

#ifdef SL_USE_VK
#include "starlight/renderer/gpu/vulkan/VKMesh.hh"
#include "starlight/renderer/gpu/vulkan/VKRendererBackend.hh"
#endif

namespace sl {

Mesh::Mesh(const Properties& props, const Data& data) :
    m_props(props), m_extent(data.extent) {}

ResourceRef<Mesh> Mesh::load(const MeshConfig2D& config) {
    return MeshManager::get().load(config);
}

ResourceRef<Mesh> Mesh::load(const MeshConfig3D& config) {
    return MeshManager::get().load(config);
}

ResourceRef<Mesh> Mesh::find(const std::string& name) {
    return MeshManager::get().find(name);
}

ResourceRef<Mesh> Mesh::getCube() {
    static CubeProperties properties{
        10.0f, 10.0f, 10.0f, 1, 1, "Internal.Mesh.SkyboxCube", ""
    };
    return load(MeshConfig3D::generateCube(properties));
}

const Mesh::BufferDescription& Mesh::getDataDescription() const {
    return m_dataDescription;
}

const Mesh::Properties& Mesh::getProperties() const { return m_props; }

const Extent3& Mesh::getExtent() const { return m_extent; }

static OwningPtr<Mesh> createMesh(
  RendererBackend& renderer, const Mesh::Data& data,
  const Mesh::Properties& properties
) {
#ifdef SL_USE_VK
    auto& vkRenderer = static_cast<vk::VKRendererBackend&>(renderer);

    return createOwningPtr<vk::VKMesh>(
      vkRenderer.getContext(), vkRenderer.getLogicalDevice(),
      vkRenderer.getVertexBuffer(), vkRenderer.getIndexBuffer(), properties, data
    );
#else
    FATAL_ERROR("Could not find renderer backend implementation");
#endif
}

MeshManager::MeshManager(RendererBackend& renderer) : m_renderer(renderer) {}

ResourceRef<Mesh> MeshManager::load(const MeshConfig2D& config) {
    if (auto resource = find(config.name); resource) return resource;

    Mesh::Data data{
        sizeof(Vertex2), config.vertices.size(),   config.vertices.data(),
        config.indices,  config.calculateExtent(),
    };
    Mesh::Properties properties{ config.name };

    return store(config.name, createMesh(m_renderer, data, properties));
}

ResourceRef<Mesh> MeshManager::load(const MeshConfig3D& config) {
    if (auto resource = find(config.name); resource) return resource;

    Mesh::Data data{
        sizeof(Vertex3), config.vertices.size(),   config.vertices.data(),
        config.indices,  config.calculateExtent(),
    };
    Mesh::Properties properties{ config.name };

    return store(config.name, createMesh(m_renderer, data, properties));
}

MeshConfig3D MeshConfig3D::generatePlane(const PlaneProperties& props) {
    MeshConfig3D out;

    const auto vertexCount = props.xSegments * props.ySegments * 4;
    const auto indexCount  = props.xSegments * props.ySegments * 6;

    out.vertices.resize(vertexCount);
    out.indices.resize(indexCount);

    float segWidth   = (float)props.width / props.xSegments;
    float segHeight  = (float)props.height / props.ySegments;
    float halfWidth  = props.width / 2.0f;
    float halfHeight = props.height / 2.0f;

    for (uint32_t y = 0; y < props.ySegments; ++y) {
        for (uint32_t x = 0; x < props.xSegments; ++x) {
            float minX = (x * segWidth) - halfWidth;
            float minY = (y * segHeight) - halfHeight;
            float maxX = minX + segWidth;
            float maxY = minY + segHeight;

            float minUVX = (x / (float)props.xSegments) * props.xTile;
            float minUVY = (y / (float)props.ySegments) * props.yTile;
            float maxUVX = ((x + 1.0f) / (float)props.xSegments) * props.xTile;
            float maxUVY = ((y + 1.0f) / (float)props.ySegments) * props.yTile;

            uint32_t vOffset = ((y * props.xSegments) + x) * 4;

            auto v0 = &out.vertices[vOffset + 0];
            auto v1 = &out.vertices[vOffset + 1];
            auto v2 = &out.vertices[vOffset + 2];
            auto v3 = &out.vertices[vOffset + 3];

            v0->position           = glm::vec3{ minX, minY, 0.0f };
            v0->textureCoordinates = glm::vec2{ minUVX, minUVY };

            v1->position           = glm::vec3{ maxX, maxY, 0.0f };
            v1->textureCoordinates = glm::vec2{ minUVX, maxUVY };

            v2->position           = glm::vec3{ minX, maxY, 0.0f };
            v2->textureCoordinates = glm::vec2{ minUVX, maxUVY };

            v3->position           = glm::vec3{ maxX, minY, 0.0f };
            v3->textureCoordinates = glm::vec2{ maxUVX, minUVY };

            uint32_t iOffset = ((y * props.xSegments) + x) * 6;

            out.indices[iOffset + 0] = vOffset + 0;
            out.indices[iOffset + 1] = vOffset + 1;
            out.indices[iOffset + 2] = vOffset + 2;
            out.indices[iOffset + 3] = vOffset + 0;
            out.indices[iOffset + 4] = vOffset + 3;
            out.indices[iOffset + 5] = vOffset + 1;
        }
    }
    out.name = props.name;

    return out;
}

MeshConfig3D MeshConfig3D::generateCube(const CubeProperties& props) {
    const auto& [width, height, depth, xTile, yTile, name, materialName] = props;

    ASSERT(
      width > 0 && depth > 0 && height > 0 && xTile > 0 && yTile > 0,
      "Invalid properties for cube, dimensions must be greater than 0"
    );

    MeshConfig3D out;

    const auto vertexCount = 4 * 6;
    const auto indexCount  = 6 * 6;

    out.vertices.resize(vertexCount);
    out.indices.resize(indexCount);

    float halfWidth  = width * 0.5f;
    float halfHeight = height * 0.5f;
    float halfDepth  = depth * 0.5f;
    float min_x      = -halfWidth;
    float min_y      = -halfHeight;
    float min_z      = -halfDepth;
    float max_x      = halfWidth;
    float max_y      = halfHeight;
    float max_z      = halfDepth;
    float min_uvx    = 0.0f;
    float min_uvy    = 0.0f;
    float max_uvx    = xTile;
    float max_uvy    = yTile;

    // Front face
    out.vertices[(0 * 4) + 0].position           = Vec3<f32>{ min_x, min_y, max_z };
    out.vertices[(0 * 4) + 1].position           = Vec3<f32>{ max_x, max_y, max_z };
    out.vertices[(0 * 4) + 2].position           = Vec3<f32>{ min_x, max_y, max_z };
    out.vertices[(0 * 4) + 3].position           = Vec3<f32>{ max_x, min_y, max_z };
    out.vertices[(0 * 4) + 0].textureCoordinates = Vec2<f32>{ min_uvx, min_uvy };
    out.vertices[(0 * 4) + 1].textureCoordinates = Vec2<f32>{ max_uvx, max_uvy };
    out.vertices[(0 * 4) + 2].textureCoordinates = Vec2<f32>{ min_uvx, max_uvy };
    out.vertices[(0 * 4) + 3].textureCoordinates = Vec2<f32>{ max_uvx, min_uvy };
    out.vertices[(0 * 4) + 0].normal             = Vec3<f32>{ 0.0f, 0.0f, 1.0f };
    out.vertices[(0 * 4) + 1].normal             = Vec3<f32>{ 0.0f, 0.0f, 1.0f };
    out.vertices[(0 * 4) + 2].normal             = Vec3<f32>{ 0.0f, 0.0f, 1.0f };
    out.vertices[(0 * 4) + 3].normal             = Vec3<f32>{ 0.0f, 0.0f, 1.0f };

    // Back face
    out.vertices[(1 * 4) + 0].position           = Vec3<f32>{ max_x, min_y, min_z };
    out.vertices[(1 * 4) + 1].position           = Vec3<f32>{ min_x, max_y, min_z };
    out.vertices[(1 * 4) + 2].position           = Vec3<f32>{ max_x, max_y, min_z };
    out.vertices[(1 * 4) + 3].position           = Vec3<f32>{ min_x, min_y, min_z };
    out.vertices[(1 * 4) + 0].textureCoordinates = Vec2<f32>{ min_uvx, min_uvy };
    out.vertices[(1 * 4) + 1].textureCoordinates = Vec2<f32>{ max_uvx, max_uvy };
    out.vertices[(1 * 4) + 2].textureCoordinates = Vec2<f32>{ min_uvx, max_uvy };
    out.vertices[(1 * 4) + 3].textureCoordinates = Vec2<f32>{ max_uvx, min_uvy };
    out.vertices[(1 * 4) + 0].normal             = Vec3<f32>{ 0.0f, 0.0f, -1.0f };
    out.vertices[(1 * 4) + 1].normal             = Vec3<f32>{ 0.0f, 0.0f, -1.0f };
    out.vertices[(1 * 4) + 2].normal             = Vec3<f32>{ 0.0f, 0.0f, -1.0f };
    out.vertices[(1 * 4) + 3].normal             = Vec3<f32>{ 0.0f, 0.0f, -1.0f };

    // Left
    out.vertices[(2 * 4) + 0].position           = Vec3<f32>{ min_x, min_y, min_z };
    out.vertices[(2 * 4) + 1].position           = Vec3<f32>{ min_x, max_y, max_z };
    out.vertices[(2 * 4) + 2].position           = Vec3<f32>{ min_x, max_y, min_z };
    out.vertices[(2 * 4) + 3].position           = Vec3<f32>{ min_x, min_y, max_z };
    out.vertices[(2 * 4) + 0].textureCoordinates = Vec2<f32>{ min_uvx, min_uvy };
    out.vertices[(2 * 4) + 1].textureCoordinates = Vec2<f32>{ max_uvx, max_uvy };
    out.vertices[(2 * 4) + 2].textureCoordinates = Vec2<f32>{ min_uvx, max_uvy };
    out.vertices[(2 * 4) + 3].textureCoordinates = Vec2<f32>{ max_uvx, min_uvy };
    out.vertices[(2 * 4) + 0].normal             = Vec3<f32>{ -1.0f, 0.0f, 0.0f };
    out.vertices[(2 * 4) + 1].normal             = Vec3<f32>{ -1.0f, 0.0f, 0.0f };
    out.vertices[(2 * 4) + 2].normal             = Vec3<f32>{ -1.0f, 0.0f, 0.0f };
    out.vertices[(2 * 4) + 3].normal             = Vec3<f32>{ -1.0f, 0.0f, 0.0f };

    // Right face
    out.vertices[(3 * 4) + 0].position           = Vec3<f32>{ max_x, min_y, max_z };
    out.vertices[(3 * 4) + 1].position           = Vec3<f32>{ max_x, max_y, min_z };
    out.vertices[(3 * 4) + 2].position           = Vec3<f32>{ max_x, max_y, max_z };
    out.vertices[(3 * 4) + 3].position           = Vec3<f32>{ max_x, min_y, min_z };
    out.vertices[(3 * 4) + 0].textureCoordinates = Vec2<f32>{ min_uvx, min_uvy };
    out.vertices[(3 * 4) + 1].textureCoordinates = Vec2<f32>{ max_uvx, max_uvy };
    out.vertices[(3 * 4) + 2].textureCoordinates = Vec2<f32>{ min_uvx, max_uvy };
    out.vertices[(3 * 4) + 3].textureCoordinates = Vec2<f32>{ max_uvx, min_uvy };
    out.vertices[(3 * 4) + 0].normal             = Vec3<f32>{ 1.0f, 0.0f, 0.0f };
    out.vertices[(3 * 4) + 1].normal             = Vec3<f32>{ 1.0f, 0.0f, 0.0f };
    out.vertices[(3 * 4) + 2].normal             = Vec3<f32>{ 1.0f, 0.0f, 0.0f };
    out.vertices[(3 * 4) + 3].normal             = Vec3<f32>{ 1.0f, 0.0f, 0.0f };

    // Bottom face
    out.vertices[(4 * 4) + 0].position           = Vec3<f32>{ max_x, min_y, max_z };
    out.vertices[(4 * 4) + 1].position           = Vec3<f32>{ min_x, min_y, min_z };
    out.vertices[(4 * 4) + 2].position           = Vec3<f32>{ max_x, min_y, min_z };
    out.vertices[(4 * 4) + 3].position           = Vec3<f32>{ min_x, min_y, max_z };
    out.vertices[(4 * 4) + 0].textureCoordinates = Vec2<f32>{ min_uvx, min_uvy };
    out.vertices[(4 * 4) + 1].textureCoordinates = Vec2<f32>{ max_uvx, max_uvy };
    out.vertices[(4 * 4) + 2].textureCoordinates = Vec2<f32>{ min_uvx, max_uvy };
    out.vertices[(4 * 4) + 3].textureCoordinates = Vec2<f32>{ max_uvx, min_uvy };
    out.vertices[(4 * 4) + 0].normal             = Vec3<f32>{ 0.0f, -1.0f, 0.0f };
    out.vertices[(4 * 4) + 1].normal             = Vec3<f32>{ 0.0f, -1.0f, 0.0f };
    out.vertices[(4 * 4) + 2].normal             = Vec3<f32>{ 0.0f, -1.0f, 0.0f };
    out.vertices[(4 * 4) + 3].normal             = Vec3<f32>{ 0.0f, -1.0f, 0.0f };

    // Top face
    out.vertices[(5 * 4) + 0].position           = Vec3<f32>{ min_x, max_y, max_z };
    out.vertices[(5 * 4) + 1].position           = Vec3<f32>{ max_x, max_y, min_z };
    out.vertices[(5 * 4) + 2].position           = Vec3<f32>{ min_x, max_y, min_z };
    out.vertices[(5 * 4) + 3].position           = Vec3<f32>{ max_x, max_y, max_z };
    out.vertices[(5 * 4) + 0].textureCoordinates = Vec2<f32>{ min_uvx, min_uvy };
    out.vertices[(5 * 4) + 1].textureCoordinates = Vec2<f32>{ max_uvx, max_uvy };
    out.vertices[(5 * 4) + 2].textureCoordinates = Vec2<f32>{ min_uvx, max_uvy };
    out.vertices[(5 * 4) + 3].textureCoordinates = Vec2<f32>{ max_uvx, min_uvy };
    out.vertices[(5 * 4) + 0].normal             = Vec3<f32>{ 0.0f, 1.0f, 0.0f };
    out.vertices[(5 * 4) + 1].normal             = Vec3<f32>{ 0.0f, 1.0f, 0.0f };
    out.vertices[(5 * 4) + 2].normal             = Vec3<f32>{ 0.0f, 1.0f, 0.0f };
    out.vertices[(5 * 4) + 3].normal             = Vec3<f32>{ 0.0f, 1.0f, 0.0f };

    for (u32 i = 0; i < 6; ++i) {
        u32 v_offset              = i * 4;
        u32 i_offset              = i * 6;
        out.indices[i_offset + 0] = v_offset + 0;
        out.indices[i_offset + 1] = v_offset + 1;
        out.indices[i_offset + 2] = v_offset + 2;
        out.indices[i_offset + 3] = v_offset + 0;
        out.indices[i_offset + 4] = v_offset + 3;
        out.indices[i_offset + 5] = v_offset + 1;
    }

    out.name = name;
    return out;
}

void MeshConfig3D::generateTangents() { sl::generateTangents(vertices, indices); }

void MeshConfig3D::generateNormals() { sl::generateFaceNormals(vertices, indices); }

}  // namespace sl