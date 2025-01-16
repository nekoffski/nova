#pragma once

#include <optional>

#include "starlight/core/Core.hh"
#include "starlight/renderer/Core.hh"

#include "Sync.hh"

namespace sl {

struct Buffer : public NonCopyable, public NonMovable {
    struct Properties {
        u64 size;
        MemoryProperty memoryProperty;
        BufferUsage usage;
        bool bindOnCreate;
    };

    struct Region {
        u64 src;
        u64 dest;
        u64 size;
    };

    virtual ~Buffer() = default;

    virtual void bind(u64 offset = 0) = 0;

    virtual void* lockMemory(
      u64 offset, u64 size, MemoryProperty memoryProperty
    )                           = 0;
    virtual void unlockMemory() = 0;

    virtual std::optional<u64> allocate(u64 size) = 0;
    virtual void free(u64 size, u64 offset)       = 0;

    virtual void loadData(
      u64 offset, u64 size, MemoryProperty memoryProperty, const void* data
    ) = 0;
};

}  // namespace sl
