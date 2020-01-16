#include "Engine/Mesh.h"

using namespace VoxelEngine;

Mesh::Mesh() {

}

void Mesh::addBinding(uint32_t vertexCount, std::shared_ptr<Buffer> buffer, vk::Format format, vk::DeviceSize offset) {
    m_bindings.push_back({ buffer, format });
    m_buffers.push_back(buffer->buffer());
    m_offsets.push_back(offset);
    m_vertexCount = vertexCount;
}

void Mesh::setIndexBuffer(uint32_t indexCount, std::shared_ptr<Buffer> buffer, vk::IndexType type, vk::DeviceSize offset) {
    m_indexBuffer = buffer;
    m_indexType = type;
    m_hasIndex = buffer != nullptr;
    m_indexOffset = offset;
    m_indexCount = indexCount;
}

void Mesh::draw(vk::CommandBuffer& commandBuffer) const {
    commandBuffer.bindVertexBuffers(0, m_buffers, m_offsets);

    if (m_hasIndex) {
        commandBuffer.bindIndexBuffer(m_indexBuffer->buffer(), 0, m_indexType);
        commandBuffer.drawIndexed(m_indexCount, 1, 0, 0, 0);
    } else {
        commandBuffer.draw(m_vertexCount, 1, 0, 0);
    }
}