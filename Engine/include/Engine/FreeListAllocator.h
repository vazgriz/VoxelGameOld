#pragma once
#include <VulkanWrapper/VulkanWrapper.h>
#include <list>

namespace VoxelEngine {
    class FreeListAllocator;

    struct Allocation {
        FreeListAllocator* allocator;
        size_t offset;
        size_t size;
    };

    class FreeListAllocator {
        struct Node {
            size_t offset;
            size_t size;
        };

    public:
        FreeListAllocator(size_t offset, size_t size);
        FreeListAllocator(const FreeListAllocator& other) = delete;
        FreeListAllocator& operator = (const FreeListAllocator& other) = delete;
        FreeListAllocator(FreeListAllocator&& other) = default;
        FreeListAllocator& operator = (FreeListAllocator&& other) = default;

        Allocation allocate(size_t size, size_t alignment);
        void free(Allocation allocation);
        void reset();

    private:
        size_t m_offset;
        size_t m_size;
        std::list<Node> m_nodes;

        size_t align(size_t ptr, size_t alignment);
        void split(std::list<Node>::iterator it, size_t offset, size_t size);
        void merge(std::list<Node>::iterator it, Allocation allocation);
        void merge(std::list<Node>::iterator front, std::list<Node>::iterator back);
    };
}