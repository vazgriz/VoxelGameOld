#include "Engine/FreeListAllocator.h"

using namespace VoxelEngine;

FreeListAllocator::FreeListAllocator(size_t offset, size_t size) {
    m_offset = offset;
    m_size = size;

    m_nodes.emplace_front(Node{ offset, size });
}

Allocation FreeListAllocator::allocate(size_t size, size_t alignment) {
    if (size > m_size) throw std::runtime_error("Allocation too large");

    for (auto it = m_nodes.begin(); it != m_nodes.end(); it++) {
        size_t start = align(it->offset, alignment);
        size_t end = start + size;

        if (end <= (it->offset + it->size)) {
            split(it, start, size);
            return { this, start, size };
        }
    }

    return {};
}

void FreeListAllocator::free(Allocation allocation) {
    if (allocation.allocator == nullptr) return;

    auto it = m_nodes.begin();

    if (it == m_nodes.end()) {
        m_nodes.push_back({ allocation.offset, allocation.size });
        return;
    }

    if (allocation.offset < it->offset) {
        m_nodes.push_front({ allocation.offset, allocation.size });
        auto begin = m_nodes.begin();
        merge(begin, it);
        return;
    }

    for (; it != m_nodes.end(); it++) {
        if (it->offset < allocation.offset) {
            merge(it, allocation);
            return;
        }
    }
}

void FreeListAllocator::reset() {
    m_nodes.clear();
    m_nodes.emplace_front(Node{ m_offset,m_size });
}

size_t FreeListAllocator::align(size_t ptr, size_t alignment) {
    size_t unalign = ptr % alignment;

    if (unalign == 0) {
        return ptr;
    } else {
        size_t align = alignment - unalign;
        return ptr + align;
    }
}

void FreeListAllocator::split(std::list<Node>::iterator it, size_t offset, size_t size) {
    size_t frontOffset = it->offset;
    size_t frontSize = offset - frontOffset;

    size_t backOffset = offset + size;
    size_t backSize = (frontOffset + it->size) - backOffset;

    if (frontSize == 0 && backSize == 0) {
        m_nodes.erase(it);
    } else if (frontSize == 0 && backSize != 0) {
        it->offset = backOffset;
        it->size = backSize;
    } else if (frontSize != 0 && backSize == 0) {
        it->offset = frontOffset;
        it->size = frontSize;
    } else {
        m_nodes.insert(it, { frontOffset, frontSize });
        it->offset = backOffset;
        it->size = backSize;
    }
}

void FreeListAllocator::merge(std::list<Node>::iterator it, Allocation allocation) {
    size_t end = it->offset + it->size;

    if (end == allocation.offset) {
        it->size += allocation.size;
        auto next = ++it;
        merge(it, next);
    } else {
        auto back = it;
        back++;
        auto mid = m_nodes.insert(back, { allocation.offset, allocation.size });
        auto front = it;
        merge(mid, back);
        merge(front, mid);
    }
}

void FreeListAllocator::merge(std::list<Node>::iterator front, std::list<Node>::iterator back) {
    size_t frontEnd = front->offset + front->size;
    if (frontEnd == back->offset) {
        front->size += back->size;
        m_nodes.erase(back);
    }
}