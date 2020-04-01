#pragma once

#include <starlight/platform/texture/Cubemap.h>

namespace starl::platform::texture::opengl {

class OpenGLCubemap : public Cubemap {
public:
    explicit OpenGLCubemap(const CubemapArgs);

    ~OpenGLCubemap() override;

    void bind() override;
    void unbind() override;

private:
    unsigned int m_cubemapId;
};
}