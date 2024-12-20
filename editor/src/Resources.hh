#pragma once

#include <vector>

#include <starlight/core/utils/Resource.hh>
#include <starlight/renderer/gpu/Mesh.hh>
#include <starlight/renderer/gpu/Texture.hh>
#include <starlight/renderer/Material.hh>
#include <starlight/ui/widgets/Image.hh>

namespace sle {

class Resources : public sl::NonCopyable, public sl::NonMovable {
public:
    template <typename T> using ResourceVector = std::vector<sl::ResourceRef<T>>;

    explicit Resources();

    ResourceVector<sl::Mesh> meshes;
    ResourceVector<sl::Material> materials;
    ResourceVector<sl::Texture> textures;

    sl::ui::ImageHandle& getImageHandle(sl::Texture* texture);

private:
    std::unordered_map<sl::u64, std::unique_ptr<sl::ui::ImageHandle>> m_imageHandles;
};

}  // namespace sle
