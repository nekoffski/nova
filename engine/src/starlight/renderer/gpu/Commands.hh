#pragma once

#include <variant>

#include "starlight/core/Core.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/renderer/gpu/Buffer.hh"

namespace sl {

struct BindVertexBufferCommand {
    Buffer& buffer;
    u64 offset;
};

struct BindIndexBufferCommand {
    Buffer& buffer;
    u64 offset;
};

struct DrawCommand {
    u32 vertexCount;
    u32 instanceCount = 1u;
    u32 firstVertex   = 0u;
    u32 firstInstance = 0u;
};

struct DrawIndexedCommand {
    u32 indexCount;
    u32 firstIndex    = 0u;
    u32 vertexOffset  = 0u;
    u32 instanceCount = 1u;
    u32 firstInstance = 0u;
};

struct SetViewportCommand {
    Vec2<u32> offset;
    Vec2<u32> size;
};

struct SetScissorsCommand {
    Vec2<u32> offset;
    Vec2<u32> size;
};

using Command = std::variant<
  BindVertexBufferCommand, BindIndexBufferCommand, DrawCommand, DrawIndexedCommand,
  SetViewportCommand, SetScissorsCommand>;

}  // namespace sl
