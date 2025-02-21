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

        static Properties staging(u64 size) {
            return Buffer::Properties{
                .size = size,
                .memoryProperty =
                  MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT
                  | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT,
                .usage        = BufferUsage::BUFFER_USAGE_TRANSFER_SRC_BIT,
                .bindOnCreate = true
            };
        }
    };

    static UniquePointer<Buffer> create(const Properties& props);

    virtual ~Buffer() = default;

    virtual void bind(u64 offset = 0) = 0;

    virtual void* lockMemory(const Range& range = Range{ 0u, max<u64>() }) = 0;
    virtual void unlockMemory()                                            = 0;

    virtual std::optional<Range> allocate(u64 size, const void* data = nullptr) = 0;
    virtual void free(const Range& range)                                       = 0;

    virtual void copy(const Range& range, const void* data) = 0;
};

}  // namespace sl
