#pragma once

#include <memory>

#include "sl/platform/fwd.h"

namespace sl::gfx::buffer {

class FrameBuffer {
public:
    struct Factory {
        virtual std::shared_ptr<FrameBuffer> create() = 0;
    };

    inline static std::unique_ptr<Factory> factory = nullptr;

    virtual ~FrameBuffer() = default;

    virtual void bindTexture(std::shared_ptr<sl::gfx::Texture>) = 0;
    virtual void bind() = 0;
    virtual void unbind() = 0;
};
}