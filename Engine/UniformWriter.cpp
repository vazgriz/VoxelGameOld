#include "Engine/UniformWriter.h"
#include "Engine/Utilities.h"

using namespace VoxelEngine;

UniformWriter::UniformWriter(void* ptr) {
    m_ptr = static_cast<char*>(ptr);
    m_offset = 0;
}

void UniformWriter::write(float value) {
    m_offset = align(m_offset, sizeof(float));
    memcpy(m_ptr + m_offset, &value, sizeof(float));
    m_offset += sizeof(float);
}

void UniformWriter::write(float* values, size_t count) {
    m_offset = align(m_offset, sizeof(glm::vec4));

    for (size_t i = 0; i < count; i++) {
        glm::vec4 data = { values[i], 0, 0, 0 };
        memcpy(m_ptr + m_offset + sizeof(glm::vec4) * i, &data, sizeof(glm::vec4));
    }

    m_offset += sizeof(glm::vec4) * count;
}

void UniformWriter::write(std::vector<float> values) {
    write(values.data(), values.size());
}

void UniformWriter::write(glm::vec2 value) {
    m_offset = align(m_offset, sizeof(glm::vec2));
    memcpy(m_ptr + m_offset, &value, sizeof(glm::vec2));
    m_offset += sizeof(glm::vec2);
}

void UniformWriter::write(glm::vec3 value) {
    m_offset = align(m_offset, sizeof(glm::vec4));
    memcpy(m_ptr + m_offset, &value, sizeof(glm::vec3));
    m_offset += sizeof(glm::vec4);
}

void UniformWriter::write(glm::vec4 value) {
    m_offset = align(m_offset, sizeof(glm::vec4));
    memcpy(m_ptr + m_offset, &value, sizeof(glm::vec4));
    m_offset += sizeof(glm::vec4);
}

void UniformWriter::write(glm::mat4 value) {
    m_offset = align(m_offset, sizeof(glm::mat4));
    memcpy(m_ptr + m_offset, &value, sizeof(glm::mat4));
    m_offset += sizeof(glm::mat4);
}