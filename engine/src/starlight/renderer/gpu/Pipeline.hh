#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/math/Core.hh"

#include "CommandBuffer.hh"
#include "fwd.hh"

namespace sl {

struct Pipeline : public NonCopyable, public NonMovable {
    struct Properties {
        static Properties createDefault();

        Rect2<u32> viewport;
        Rect2<u32> scissor;
        PolygonMode polygonMode;
        CullMode cullMode;
        bool depthTestEnabled;
    };

    static UniquePointer<Pipeline> create(
      Shader& shader, RenderPassBackend& renderPass,
      const Properties& props = Properties::createDefault()
    );

    virtual ~Pipeline() = default;

    virtual void bind(CommandBuffer& commandBuffer) = 0;
};

}  // namespace sl
