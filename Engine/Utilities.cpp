#include "Engine/Utilities.h"

using namespace VoxelEngine;

size_t VoxelEngine::align(size_t ptr, size_t alignment) {
    size_t mask = alignment - 1;
    size_t tail = ptr & mask;

    if (tail == 0) {
        return ptr;
    } else {
        size_t unalign = alignment - tail;
        return ptr + unalign;
    }
}