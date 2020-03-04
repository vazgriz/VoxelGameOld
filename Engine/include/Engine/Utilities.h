#pragma once
#include <vector>
#include <string>

namespace VoxelEngine {
    size_t align(size_t ptr, size_t alignment);
    std::vector<char> readFile(const std::string& filename);
}