#include "Engine/Utilities.h"
#include <fstream>

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

std::vector<char> VoxelEngine::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

vk::ShaderModule VoxelEngine::createShaderModule(vk::Device& device, const std::vector<char>& byteCode) {
    vk::ShaderModuleCreateInfo info = {};
    info.code = byteCode;

    return vk::ShaderModule(device, info);
}