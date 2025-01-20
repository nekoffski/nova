#pragma once

#include <vector>
#include <unordered_set>

#include <starlight/core/utils/Resource.hh>
#include <starlight/renderer/Mesh.hh>
#include <starlight/renderer/gpu/Texture.hh>
#include <starlight/renderer/Material.hh>
#include <starlight/ui/widgets/Image.hh>

namespace sle {

class Resources : public sl::NonCopyable, public sl::NonMovable {
public:
    template <typename T>
    using ResourceSet =
      std::unordered_set<sl::ResourceRef<T>, typename sl::ResourceRef<T>::Hash>;

    explicit Resources();

    sl::ResourceRef<sl::Mesh> addMesh();
    sl::ResourceRef<sl::Material> addMaterial();
    sl::ResourceRef<sl::Texture> addTexture();

    sl::ui::ImageHandle& getImageHandle(sl::Texture* texture);

    void sync();

private:
    std::unordered_map<sl::u64, std::unique_ptr<sl::ui::ImageHandle>> m_imageHandles;
    // ref sets so the resources aren't released
    ResourceSet<sl::Mesh> m_meshes;
    ResourceSet<sl::Material> m_materials;
    ResourceSet<sl::Texture> m_textures;
};

}  // namespace sle
