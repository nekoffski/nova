#pragma once

#include <starlight/renderer/gpu/Texture.hh>

#include "Resources.hh"

namespace sle {

class TextureUI {
public:
    void render(sl::ResourceRef<sl::Texture> texture);
};

}  // namespace sle
