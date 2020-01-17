#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace VoxelEngine {
    class UniformWriter {
    public:
        UniformWriter(void* ptr);

        void write(float value);
        void write(float* values, size_t count);
        void write(std::vector<float> values);
        void write(glm::vec2 value);
        void write(glm::vec3 value);
        void write(glm::vec4 value);
        void write(glm::mat4 value);

    private:
        char* m_ptr;
        size_t m_offset;
    };
}