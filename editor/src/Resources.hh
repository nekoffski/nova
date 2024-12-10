#pragma once

#include <vector>

#include <starlight/core/utils/Resource.hh>
#include <starlight/renderer/gpu/Mesh.hh>
#include <starlight/renderer/gpu/Texture.hh>
#include <starlight/renderer/Material.hh>

namespace sle {

struct Resources {
    template <typename T> using ResourceVector = std::vector<sl::ResourceRef<T>>;

    explicit Resources();

    ResourceVector<sl::Mesh> meshes;
    ResourceVector<sl::Material> materials;
    ResourceVector<sl::Texture> textures;
};

}  // namespace sle
