#include "Mesh.hh"

#include "starlight/core/math/Geometry.hh"
#include "starlight/core/math/Vertex.hh"

namespace sl {

Mesh::Mesh(const Data& data, Buffer& vertexBuffer, Buffer& indexBuffer) :
    m_extent(data.extent), m_vertexBuffer(vertexBuffer), m_indexBuffer(indexBuffer) {
    LOG_DEBUG(
      "Creating Mesh, vertex data size = {}b, index data size = {}b",
      data.vertexDataSize, data.indexDataSize
    )
    auto vertexRange = vertexBuffer.allocate(data.vertexDataSize, data.vertexData);
    auto indexRange  = indexBuffer.allocate(data.indexDataSize, data.indexData);
    ASSERT(vertexRange, "Could not allocate vertex buffer range for mesh");
    ASSERT(indexRange, "Could not allocate index buffer range for mesh");

    m_memoryLayout.vertexBufferRange = *vertexRange;
    m_memoryLayout.indexBufferRange  = *indexRange;
    m_memoryLayout.indexCount        = data.indexCount;
}

Mesh::~Mesh() {
    // m_vertexBuffer.free(m_memoryLayout.vertexBufferRange);
    // m_indexBuffer.free(m_memoryLayout.indexBufferRange);
}

const Mesh::MemoryLayout& Mesh::getMemoryLayout() const { return m_memoryLayout; }

const Extent3& Mesh::getExtent() const { return m_extent; }

Mesh::Properties3D::Properties3D(const SphereProperties& props) {
    for (u32 i = 0; i <= props.stacks; ++i) {
        float V   = (float)i / (float)props.stacks;
        float phi = V * pi;

        for (u32 j = 0; j <= props.slices; ++j) {
            float U     = (float)j / (float)props.slices;
            float theta = U * (pi * 2);

            float x = props.radius * cos(theta) * sin(phi);
            float y = props.radius * cos(phi);
            float z = props.radius * sin(theta) * sin(phi);

            Vertex3 v;

            v.position.x = x;
            v.position.y = y;
            v.position.z = z;

            v.normal = math::normalize(v.position);

            v.textureCoordinates.x =
              std::atan2(v.normal.x, v.normal.z) / (2.0f * pi) + 0.5f;
            v.textureCoordinates.y = v.normal.y * 0.5f + 0.5f;

            vertices.push_back(v);
        }
    }
    for (u32 i = 0; i < props.slices * props.stacks + props.slices; ++i) {
        indices.push_back(i);
        indices.push_back(i + props.slices + 1);
        indices.push_back(i + props.slices);

        indices.push_back(i + props.slices + 1);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    this->generateTangents();
    this->generateNormals();
}

Mesh::Properties3D::Properties3D(const PlaneProperties& props) {
    const auto vertexCount = props.xSegments * props.zSegments * 4;
    const auto indexCount  = props.xSegments * props.zSegments * 6;

    vertices.resize(vertexCount);
    indices.resize(indexCount);

    float segWidth   = (float)props.width / props.xSegments;
    float segHeight  = (float)props.height / props.zSegments;
    float halfWidth  = props.width / 2.0f;
    float halfHeight = props.height / 2.0f;

    for (uint32_t y = 0; y < props.zSegments; ++y) {
        for (uint32_t x = 0; x < props.xSegments; ++x) {
            float maxX = (x * segWidth) - halfWidth;
            float minZ = (y * segHeight) - halfHeight;
            float minX = maxX + segWidth;
            float maxZ = minZ + segHeight;

            float minUVX = (x / (float)props.xSegments);
            float minUVZ = (y / (float)props.zSegments);
            float maxUVX = ((x + 1.0f) / (float)props.xSegments);
            float maxUVZ = ((y + 1.0f) / (float)props.zSegments);

            uint32_t vOffset = ((y * props.xSegments) + x) * 4;

            auto v0 = &vertices[vOffset + 0];
            auto v1 = &vertices[vOffset + 1];
            auto v2 = &vertices[vOffset + 2];
            auto v3 = &vertices[vOffset + 3];

            v0->position           = glm::vec3{ minX, 0.0f, minZ };
            v0->textureCoordinates = glm::vec2{ minUVX, minUVZ };

            v1->position           = glm::vec3{ maxX, 0.0f, maxZ };
            v1->textureCoordinates = glm::vec2{ maxUVX, maxUVZ };

            v2->position           = glm::vec3{ minX, 0.0f, maxZ };
            v2->textureCoordinates = glm::vec2{ minUVX, maxUVZ };

            v3->position           = glm::vec3{ maxX, 0.0f, minZ };
            v3->textureCoordinates = glm::vec2{ maxUVX, minUVZ };

            LOG_DEBUG("Plane y={}/x={} segment", y, x);
            LOG_DEBUG("v0={}", *v0);
            LOG_DEBUG("v1={}", *v1);
            LOG_DEBUG("v2={}", *v2);
            LOG_DEBUG("v3={}", *v3);

            uint32_t iOffset = ((y * props.xSegments) + x) * 6;

            indices[iOffset + 0] = vOffset + 0;
            indices[iOffset + 1] = vOffset + 1;
            indices[iOffset + 2] = vOffset + 2;
            indices[iOffset + 3] = vOffset + 0;
            indices[iOffset + 4] = vOffset + 3;
            indices[iOffset + 5] = vOffset + 1;
        }
    }
    this->generateTangents();
    this->generateNormals();
}

Mesh::Properties3D::Properties3D(const CubeProperties& props) {
    const auto& [width, height, depth, xTile, yTile] = props;

    ASSERT(
      width > 0 && depth > 0 && height > 0 && xTile > 0 && yTile > 0,
      "Invalid properties for cube, dimensions must be greater than 0"
    );

    const auto vertexCount = 4 * 6;
    const auto indexCount  = 6 * 6;

    vertices.resize(vertexCount);
    indices.resize(indexCount);

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
    vertices[(0 * 4) + 0].position           = Vec3<f32>{ min_x, min_y, max_z };
    vertices[(0 * 4) + 1].position           = Vec3<f32>{ max_x, max_y, max_z };
    vertices[(0 * 4) + 2].position           = Vec3<f32>{ min_x, max_y, max_z };
    vertices[(0 * 4) + 3].position           = Vec3<f32>{ max_x, min_y, max_z };
    vertices[(0 * 4) + 0].textureCoordinates = Vec2<f32>{ min_uvx, min_uvy };
    vertices[(0 * 4) + 1].textureCoordinates = Vec2<f32>{ max_uvx, max_uvy };
    vertices[(0 * 4) + 2].textureCoordinates = Vec2<f32>{ min_uvx, max_uvy };
    vertices[(0 * 4) + 3].textureCoordinates = Vec2<f32>{ max_uvx, min_uvy };
    vertices[(0 * 4) + 0].normal             = Vec3<f32>{ 0.0f, 0.0f, 1.0f };
    vertices[(0 * 4) + 1].normal             = Vec3<f32>{ 0.0f, 0.0f, 1.0f };
    vertices[(0 * 4) + 2].normal             = Vec3<f32>{ 0.0f, 0.0f, 1.0f };
    vertices[(0 * 4) + 3].normal             = Vec3<f32>{ 0.0f, 0.0f, 1.0f };

    // Back face
    vertices[(1 * 4) + 0].position           = Vec3<f32>{ max_x, min_y, min_z };
    vertices[(1 * 4) + 1].position           = Vec3<f32>{ min_x, max_y, min_z };
    vertices[(1 * 4) + 2].position           = Vec3<f32>{ max_x, max_y, min_z };
    vertices[(1 * 4) + 3].position           = Vec3<f32>{ min_x, min_y, min_z };
    vertices[(1 * 4) + 0].textureCoordinates = Vec2<f32>{ min_uvx, min_uvy };
    vertices[(1 * 4) + 1].textureCoordinates = Vec2<f32>{ max_uvx, max_uvy };
    vertices[(1 * 4) + 2].textureCoordinates = Vec2<f32>{ min_uvx, max_uvy };
    vertices[(1 * 4) + 3].textureCoordinates = Vec2<f32>{ max_uvx, min_uvy };
    vertices[(1 * 4) + 0].normal             = Vec3<f32>{ 0.0f, 0.0f, -1.0f };
    vertices[(1 * 4) + 1].normal             = Vec3<f32>{ 0.0f, 0.0f, -1.0f };
    vertices[(1 * 4) + 2].normal             = Vec3<f32>{ 0.0f, 0.0f, -1.0f };
    vertices[(1 * 4) + 3].normal             = Vec3<f32>{ 0.0f, 0.0f, -1.0f };

    // Left
    vertices[(2 * 4) + 0].position           = Vec3<f32>{ min_x, min_y, min_z };
    vertices[(2 * 4) + 1].position           = Vec3<f32>{ min_x, max_y, max_z };
    vertices[(2 * 4) + 2].position           = Vec3<f32>{ min_x, max_y, min_z };
    vertices[(2 * 4) + 3].position           = Vec3<f32>{ min_x, min_y, max_z };
    vertices[(2 * 4) + 0].textureCoordinates = Vec2<f32>{ min_uvx, min_uvy };
    vertices[(2 * 4) + 1].textureCoordinates = Vec2<f32>{ max_uvx, max_uvy };
    vertices[(2 * 4) + 2].textureCoordinates = Vec2<f32>{ min_uvx, max_uvy };
    vertices[(2 * 4) + 3].textureCoordinates = Vec2<f32>{ max_uvx, min_uvy };
    vertices[(2 * 4) + 0].normal             = Vec3<f32>{ -1.0f, 0.0f, 0.0f };
    vertices[(2 * 4) + 1].normal             = Vec3<f32>{ -1.0f, 0.0f, 0.0f };
    vertices[(2 * 4) + 2].normal             = Vec3<f32>{ -1.0f, 0.0f, 0.0f };
    vertices[(2 * 4) + 3].normal             = Vec3<f32>{ -1.0f, 0.0f, 0.0f };

    // Right face
    vertices[(3 * 4) + 0].position           = Vec3<f32>{ max_x, min_y, max_z };
    vertices[(3 * 4) + 1].position           = Vec3<f32>{ max_x, max_y, min_z };
    vertices[(3 * 4) + 2].position           = Vec3<f32>{ max_x, max_y, max_z };
    vertices[(3 * 4) + 3].position           = Vec3<f32>{ max_x, min_y, min_z };
    vertices[(3 * 4) + 0].textureCoordinates = Vec2<f32>{ min_uvx, min_uvy };
    vertices[(3 * 4) + 1].textureCoordinates = Vec2<f32>{ max_uvx, max_uvy };
    vertices[(3 * 4) + 2].textureCoordinates = Vec2<f32>{ min_uvx, max_uvy };
    vertices[(3 * 4) + 3].textureCoordinates = Vec2<f32>{ max_uvx, min_uvy };
    vertices[(3 * 4) + 0].normal             = Vec3<f32>{ 1.0f, 0.0f, 0.0f };
    vertices[(3 * 4) + 1].normal             = Vec3<f32>{ 1.0f, 0.0f, 0.0f };
    vertices[(3 * 4) + 2].normal             = Vec3<f32>{ 1.0f, 0.0f, 0.0f };
    vertices[(3 * 4) + 3].normal             = Vec3<f32>{ 1.0f, 0.0f, 0.0f };

    // Bottom face
    vertices[(4 * 4) + 0].position           = Vec3<f32>{ max_x, min_y, max_z };
    vertices[(4 * 4) + 1].position           = Vec3<f32>{ min_x, min_y, min_z };
    vertices[(4 * 4) + 2].position           = Vec3<f32>{ max_x, min_y, min_z };
    vertices[(4 * 4) + 3].position           = Vec3<f32>{ min_x, min_y, max_z };
    vertices[(4 * 4) + 0].textureCoordinates = Vec2<f32>{ min_uvx, min_uvy };
    vertices[(4 * 4) + 1].textureCoordinates = Vec2<f32>{ max_uvx, max_uvy };
    vertices[(4 * 4) + 2].textureCoordinates = Vec2<f32>{ min_uvx, max_uvy };
    vertices[(4 * 4) + 3].textureCoordinates = Vec2<f32>{ max_uvx, min_uvy };
    vertices[(4 * 4) + 0].normal             = Vec3<f32>{ 0.0f, -1.0f, 0.0f };
    vertices[(4 * 4) + 1].normal             = Vec3<f32>{ 0.0f, -1.0f, 0.0f };
    vertices[(4 * 4) + 2].normal             = Vec3<f32>{ 0.0f, -1.0f, 0.0f };
    vertices[(4 * 4) + 3].normal             = Vec3<f32>{ 0.0f, -1.0f, 0.0f };

    // Top face
    vertices[(5 * 4) + 0].position           = Vec3<f32>{ min_x, max_y, max_z };
    vertices[(5 * 4) + 1].position           = Vec3<f32>{ max_x, max_y, min_z };
    vertices[(5 * 4) + 2].position           = Vec3<f32>{ min_x, max_y, min_z };
    vertices[(5 * 4) + 3].position           = Vec3<f32>{ max_x, max_y, max_z };
    vertices[(5 * 4) + 0].textureCoordinates = Vec2<f32>{ min_uvx, min_uvy };
    vertices[(5 * 4) + 1].textureCoordinates = Vec2<f32>{ max_uvx, max_uvy };
    vertices[(5 * 4) + 2].textureCoordinates = Vec2<f32>{ min_uvx, max_uvy };
    vertices[(5 * 4) + 3].textureCoordinates = Vec2<f32>{ max_uvx, min_uvy };
    vertices[(5 * 4) + 0].normal             = Vec3<f32>{ 0.0f, 1.0f, 0.0f };
    vertices[(5 * 4) + 1].normal             = Vec3<f32>{ 0.0f, 1.0f, 0.0f };
    vertices[(5 * 4) + 2].normal             = Vec3<f32>{ 0.0f, 1.0f, 0.0f };
    vertices[(5 * 4) + 3].normal             = Vec3<f32>{ 0.0f, 1.0f, 0.0f };

    for (u32 i = 0; i < 6; ++i) {
        u32 v_offset          = i * 4;
        u32 i_offset          = i * 6;
        indices[i_offset + 0] = v_offset + 0;
        indices[i_offset + 1] = v_offset + 1;
        indices[i_offset + 2] = v_offset + 2;
        indices[i_offset + 3] = v_offset + 0;
        indices[i_offset + 4] = v_offset + 3;
        indices[i_offset + 5] = v_offset + 1;
    }
    this->generateTangents();
    this->generateNormals();
}

void Mesh::Properties3D::generateTangents() {
    sl::generateTangents(vertices, indices);
}

void Mesh::Properties3D::generateNormals() {
    sl::generateFaceNormals(vertices, indices);
}

}  // namespace sl
