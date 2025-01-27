#pragma once

#include <vector>

#include "starlight/core/containers/FreeList.hh"

namespace sl {

class DynamicAllocator {
    using Byte = char;

public:
    explicit DynamicAllocator(uint64_t size);

    void* allocate(uint64_t size);
    void free(void* block, uint64_t size);

    uint64_t spaceLeft();

private:
    uint64_t m_totalSize;

    FreeList m_freeList;
    std::vector<Byte> m_memory;
    char* m_memoryAlias;
};

}  // namespace sl