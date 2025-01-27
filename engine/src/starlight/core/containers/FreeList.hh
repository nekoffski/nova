#pragma once

#include <vector>
#include <cstdint>
#include <optional>

#include "starlight/core/Core.hh"
#include "starlight/core/Log.hh"

namespace sl {

class FreeList {
    struct Node {
        explicit Node();

        void invalidate();

        u64 offset;
        u64 size;
        Node* next;
    };

public:
    explicit FreeList(u64 size);

    void freeBlock(u64 size, u64 offset);

    std::optional<u64> allocateBlock(u64 size);
    u64 spaceLeft();

    void resize(u64 newSize);

    void clear();

private:
    Node* getFreeNode();

    u64 m_totalSize;
    u64 m_maxEntries;

    std::vector<Node> m_nodes;
    Node* m_head;
};

}  // namespace sl
