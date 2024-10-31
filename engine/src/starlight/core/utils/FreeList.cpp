#include "FreeList.hh"

namespace sl {

FreeList::Node::Node() { invalidate(); }

// TODO: ugly, refactor
static constexpr auto invalidId = std::numeric_limits<u64>::max();

void FreeList::Node::invalidate() {
    offset = invalidId;
    size   = invalidId;
    next   = nullptr;
}

FreeList::FreeList(u64 size) :
    m_totalSize(size), m_maxEntries(size / sizeof(void*)), m_nodes(m_maxEntries),
    m_head(&m_nodes[0]) {
    m_head->offset = 0;
    m_head->size   = m_totalSize;

    LOG_TRACE(
      "Creating free list with {}b capacity, maxEntries={}", m_totalSize,
      m_maxEntries
    );
}

void FreeList::resize(u64 newSize) {
    m_maxEntries = newSize / sizeof(void*);

    u64 sizeDiff = newSize - m_totalSize;
    ASSERT(sizeDiff > 0, "New size of the list must be greater than the actual one");

    m_totalSize = newSize;

    m_nodes.clear();
    m_nodes.resize(newSize);

    m_head = &m_nodes[0];
    // TODO
    ASSERT(false, "NOT IMPLEMENTED YET!");
}

void FreeList::freeBlock(u64 size, u64 offset) {
    ASSERT(
      size > 0 && offset > 0,
      "Could not free block with invalid offset ({}) or size ({})", offset, size
    );

    Node* previous = nullptr;
    Node* node     = m_head;

    while (node != nullptr) {
        if (node->offset == offset) {
            node->size += size;

            if (node->next && node->next->offset == node->offset + node->size) {
                node->size += node->next->size;
                auto next  = node->next;
                node->next = next->next;
                next->invalidate();
            }
        } else if (node->offset > offset) {
            Node* newNode = getFreeNode();

            newNode->offset = offset;
            newNode->size   = size;
            newNode->next   = node;

            if (previous)
                previous->next = newNode;
            else
                m_head = newNode;

            if (newNode->next
                && newNode->offset + newNode->size == newNode->next->offset) {
                newNode->size += newNode->next->size;
                auto rubbish  = newNode->next;
                newNode->next = rubbish->next;
                rubbish->invalidate();
            }
            if (previous && previous->offset + previous->size == newNode->offset) {
                previous->size += newNode->size;
                previous->next = newNode->next;
                newNode->invalidate();
            }
        }
        previous = node;
        node     = node->next;
    }
    LOG_WARN("Unable to find block to free, that's unexpected");
}

std::optional<u64> FreeList::allocateBlock(u64 size) {
    ASSERT(size > 0, "Could not allocate block with size less or equal 0");

    Node* previous = nullptr;
    Node* node     = m_head;

    while (node != nullptr) {
        if (node->size == size) {
            u64 offset = node->offset;

            if (previous) {
                previous->next = node->next;
                node->invalidate();
            } else {
                m_head->invalidate();
                m_head = node->next;
            }

            return offset;
        } else if (node->size > size) {
            u64 offset = node->offset;

            node->size -= size;
            node->offset += size;

            return offset;
        }
        previous = node;
        node     = node->next;
    }

    LOG_WARN(
      "Could not find block with enough memory {} bytes requested, total space left: {}",
      size, spaceLeft()
    );
    return {};
}

u64 FreeList::spaceLeft() {
    u64 totalSpace = 0u;

    for (Node* node = m_head; node != nullptr; node = node->next)
        totalSpace += node->size;

    return totalSpace;
}

void FreeList::clear() {
    for (u64 i = 0; i < m_maxEntries; ++i) m_nodes[i].invalidate();

    m_head->offset = 0;
    m_head->size   = m_totalSize;
    m_head->next   = nullptr;
}

FreeList::Node* FreeList::getFreeNode() {
    for (u64 i = 0; i < m_maxEntries; ++i)
        if (m_nodes[i].offset == invalidId) return &m_nodes[i];
    return nullptr;
}

}  // namespace sl
