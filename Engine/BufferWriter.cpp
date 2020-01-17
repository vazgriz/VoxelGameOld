#include "Engine/BufferWriter.h"
#include "Engine/Utilities.h"

using namespace VoxelEngine;

BufferWriter::BufferWriter(void* ptr) {
    m_ptr = static_cast<char*>(ptr);
    m_offset = 0;
}

void BufferWriter::write(float value) {
    m_offset = align(m_offset, sizeof(float));
    memcpy(m_ptr + m_offset, &value, sizeof(float));
    m_offset += sizeof(float);
}

void BufferWriter::write(float* values, size_t count) {
    m_offset = align(m_offset, sizeof(float));
    memcpy(m_ptr + m_offset, values, count);
    m_offset += sizeof(float) * count;
}

void BufferWriter::write(std::vector<float> values) {
    write(values.data(), values.size());
}

void BufferWriter::write(glm::vec2 value) {
    m_offset = align(m_offset, sizeof(glm::vec2));
    memcpy(m_ptr + m_offset, &value, sizeof(glm::vec2));
    m_offset += sizeof(glm::vec2);
}

void BufferWriter::write(glm::vec3 value) {
    m_offset = align(m_offset, sizeof(glm::vec4));
    memcpy(m_ptr + m_offset, &value, sizeof(glm::vec3));
    m_offset += sizeof(glm::vec4);
}

void BufferWriter::write(glm::vec4 value) {
    m_offset = align(m_offset, sizeof(glm::vec4));
    memcpy(m_ptr + m_offset, &value, sizeof(glm::vec4));
    m_offset += sizeof(glm::vec4);
}

void BufferWriter::write(glm::mat4 value) {
    m_offset = align(m_offset, sizeof(glm::mat4));
    memcpy(m_ptr + m_offset, &value, sizeof(glm::mat4));
    m_offset += sizeof(glm::mat4);
}