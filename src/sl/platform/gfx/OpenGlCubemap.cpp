#include "OpenGlCubemap.h"

#include "Utils.hpp"
#include "sl/core/Errors.hpp"
#include "sl/core/Logger.h"
#include "sl/gfx/Image.h"

#include <glad/glad.h>

namespace sl::platform::gfx {

OpenGlCubemap::OpenGlCubemap(const sl::gfx::CubemapArgs& faces)
    : m_faces(faces)
    , m_cubemapId(0u) {
    glGenTextures(1, &m_cubemapId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapId);

    SL_INFO("Loading cubemap");

    const auto facesLen = faces.size();
    for (int i = 0; i < facesLen; ++i) {
        const auto img = sl::gfx::Image::factory->create(faces[i]);
        const auto format = channelsToFormat.find(img->getChannels());
        if (format == channelsToFormat.end())
            throw core::TextureError { core::ErrorCode::UnknownTextureFormat };
        const auto& size = img->getSize();

        SL_DEBUG("Face: {}, width: {}, height: {}", faces[i], size.width, size.height);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format->second, size.width, size.height,
            0, format->second, GL_UNSIGNED_BYTE, img->getRaw());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

OpenGlCubemap::~OpenGlCubemap() {
    if (m_cubemapId)
        glDeleteTextures(1, &m_cubemapId);
}

void OpenGlCubemap::bind() {
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapId);
}

void OpenGlCubemap::unbind() {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0u);
}
}