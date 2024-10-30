#include "DynamicAllocator.hh"

namespace sl {

DynamicAllocator::DynamicAllocator(uint64_t size) :
    m_totalSize(size), m_freeList(size), m_memory(size, 0),
    m_memoryAlias(m_memory.data()) {
    ASSERT(size > 0, "Could not created allocator with size=0");
}

void* DynamicAllocator::allocate(uint64_t size) {
    ASSERT(size > 0, "Could not allocate memory block of size 0");

    if (auto offset = m_freeList.allocateBlock(size); offset.has_value()) {
        void* block = static_cast<void*>(m_memoryAlias + *offset);
        LOG_TRACE("Allocating block of size {} at {}", size, block);
        return block;
    }

    LOG_ERROR("Could not allocate memory, not enough left");
    return nullptr;
}

void DynamicAllocator::free(void* block, uint64_t size) {
    ASSERT(block != nullptr, "Could not free nullptr");
    LOG_TRACE("Freeing block {} of size {}", block, size);

    if (block < m_memoryAlias || block > m_memoryAlias + m_totalSize) {
        LOG_ERROR(
          "Trying to free block {} outside of allocator range {} - {}", block,
          m_memoryAlias, m_memoryAlias + m_totalSize
        );
        return;
    }

    uint64_t offset = static_cast<char*>(block) - m_memoryAlias;
    m_freeList.freeBlock(size, offset);
}

uint64_t DynamicAllocator ::spaceLeft() { return m_freeList.spaceLeft(); }

}  // namespace sl
