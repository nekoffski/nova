#pragma once

#include "starlight/core/Core.hh"

namespace sl {

struct Allocator {
    [[nodiscard]] virtual void* allocate(u64 n)        = 0;
    virtual void deallocate(void* ptr, u64 n) noexcept = 0;
};

}  // namespace sl
