#pragma once

#include <vector>
#include <unordered_set>

#include <starlight/renderer/Mesh.hh>
#include <starlight/renderer/gpu/Texture.hh>
#include <starlight/renderer/Material.hh>
#include <starlight/ui/widgets/Image.hh>

namespace sle {

// TODO: is this required?
class Resources : public sl::NonCopyable, public sl::NonMovable {
public:
    struct Hash {
        template <typename T>
        std::size_t operator()(sl::SharedPtr<T> const& p) const {
            return std::hash<const T*>()(p.get());
        }
    };

    template <typename T>
    using ResourceSet = std::unordered_set<sl::SharedPtr<T>, Hash>;

    explicit Resources();

    sl::SharedPtr<sl::Mesh> addMesh();
    sl::SharedPtr<sl::Material> addMaterial();
    sl::SharedPtr<sl::Texture> addTexture();

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
