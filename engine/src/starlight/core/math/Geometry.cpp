#include "Geometry.hh"

namespace sl {

void generateFaceNormals(std::span<Vertex3> vertices, const std::span<u32> indices) {
    const int indexCount = indices.size();
    ASSERT(
      indexCount % 3 == 0, "Index count ({}) must be a multiplication of 3",
      indexCount
    );

    for (int i = 0; i < indexCount; i += 3) {
        auto& v0 = vertices[indices[i]];
        auto& v1 = vertices[indices[i + 1]];
        auto& v2 = vertices[indices[i + 2]];

        auto edge1 = v1.position - v0.position;
        auto edge2 = v2.position - v0.position;

        auto normal = glm::normalize(glm::cross(edge1, edge2));

        v0.normal = normal;
        v1.normal = normal;
        v2.normal = normal;
    }
}

void generateTangents(std::span<Vertex3> vertices, const std::span<u32> indices) {
    const int indexCount = indices.size();
    ASSERT(
      indexCount % 3 == 0, "Index count ({}) must be a multiplication of 3",
      indexCount
    );

    for (int i = 0; i < indexCount; i += 3) {
        auto& v0 = vertices[indices[i]];
        auto& v1 = vertices[indices[i + 1]];
        auto& v2 = vertices[indices[i + 2]];

        // LOG_TRACE(
        //   "Input vertices positions: {} {} {}", v0.position, v1.position,
        //   v2.position
        // );
        // LOG_TRACE(
        //   "Input vertices tex coords: {} {} {}", v0.textureCoordinates,
        //   v1.textureCoordinates, v2.textureCoordinates
        // );

        auto edge1 = v1.position - v0.position;
        auto edge2 = v2.position - v0.position;

        auto deltaUV1 = v1.textureCoordinates - v0.textureCoordinates;
        auto deltaUV2 = v2.textureCoordinates - v0.textureCoordinates;

        float dividend = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        float fc       = 1.0f / dividend;

        // LOG_TRACE("fc={}", fc);

        auto tangent = Vec3<f32>(
          deltaUV2.y * edge1.x - deltaUV1.y * edge2.x,
          deltaUV2.y * edge1.y - deltaUV1.y * edge2.y,
          deltaUV2.y * edge1.z - deltaUV1.y * edge2.z
        );
        tangent = glm::normalize(fc * tangent);

        float handedness =
          (deltaUV1.y * deltaUV2.x - deltaUV2.y * deltaUV1.x) < 0.0f ? -1.0f : 1.0f;
        auto hTangent = Vec4<f32>(tangent, handedness);

        v0.tangent = hTangent;
        v1.tangent = hTangent;
        v2.tangent = hTangent;

        // LOG_TRACE("Generated tangent: {}", hTangent);
    }
}

}  // namespace sl
