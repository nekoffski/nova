#pragma once

#include <string>
#include <span>

#include <kc/core/Meta.hpp>

#include "fwd.hh"

#include "starlight/core/math/Extent.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/core/math/Vertex.hh"
#include "starlight/core/utils/Resource.hh"

#include "gpu/Device.hh"
#include "gpu/Buffer.hh"

namespace sl {
namespace detail {

template <typename T>
concept ExtentType = kc::core::is_one_of2_v<T, Extent2, Extent3>;

template <typename T>
concept VertexType = kc::core::is_one_of2_v<T, Vertex2, Vertex3>;

template <ExtentType Extent, VertexType Vertex>
Extent calculateExtent(const std::vector<Vertex>& vertices) {
    log::expect(vertices.size() > 0, "vertices.size() == 0");
    using Vector = std::remove_cv_t<typeof(vertices[0].position)>;

    Vector min(0.0f);
    Vector max(0.0f);

    constexpr auto components = []() {
        if constexpr (std::is_same_v<Vertex, Vertex2>)
            return 2u;
        else
            return 3u;
    }();

    for (const auto& vertex : vertices) {
        for (u8 i = 0; i < components; ++i) {
            const auto& component = vertex.position[i];
            if (component < min[i]) min[i] = component;
            if (component > max[i]) max[i] = component;
        }
    }

    return Extent(min, max);
}

}  // namespace detail

struct PlaneProperties {
    float width;
    float height;
    u32 xSegments;
    u32 zSegments;
};

struct CubeProperties {
    float width;
    float height;
    float depth;
    u32 xTile;
    u32 yTile;
};

struct SphereProperties {
    u32 stacks;
    u32 slices;
    float radius;
};

class Mesh : public NonMovable, public Identificable<Mesh> {
public:
    struct Data {
        u64 indexCount;
        u64 vertexDataSize;
        u64 indexDataSize;

        const void* vertexData;
        const void* indexData;

        Extent3 extent;
    };

    struct MemoryLayout {
        Range vertexBufferRange;
        Range indexBufferRange;
        u64 indexCount;
    };

    template <detail::VertexType Vertex, detail::ExtentType Extent>
    struct Properties {
        using IndexType = u32;

        std::vector<IndexType> indices;
        std::vector<Vertex> vertices;

        Extent calculateExtent() const {
            return detail::calculateExtent<Extent, Vertex>(vertices);
        }

        Data toMeshData() const& {
            return Data{
                .indexCount     = indices.size(),
                .vertexDataSize = sizeof(Vertex) * vertices.size(),
                .indexDataSize  = sizeof(IndexType) * indices.size(),
                .vertexData     = vertices.data(),
                .indexData      = indices.data(),
                .extent         = calculateExtent(),
            };
        }
    };

    struct Properties3D final : public Properties<Vertex3, Extent3> {
        explicit Properties3D() = default;
        explicit Properties3D(const PlaneProperties& props);
        explicit Properties3D(const CubeProperties& props);
        explicit Properties3D(const SphereProperties& props);

        void generateTangents();
        void generateNormals();
    };

    struct Properties2D final : public Properties<Vertex2, Extent2> {};

    explicit Mesh(const Data& data, Buffer& vertexBuffer, Buffer& indexBuffer);
    ~Mesh();

    const MemoryLayout& getMemoryLayout() const;
    const Extent3& getExtent() const;

protected:
    Extent3 m_extent;
    Buffer& m_vertexBuffer;
    Buffer& m_indexBuffer;
    MemoryLayout m_memoryLayout;
};

}  // namespace sl
