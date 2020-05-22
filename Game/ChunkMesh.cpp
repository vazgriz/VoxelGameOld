#include "ChunkMesh.h"

ChunkMesh::ChunkMesh() {

}

void ChunkMesh::clearMesh() {
    m_mesh.setIndexCount(0);
}

void ChunkMesh::clearBindings() {
    m_mesh.clearBindings();
    m_allocations.clear();
}

void ChunkMesh::addBinding(MeshAllocation&& allocation) {
    auto& alloc = m_allocations.emplace_back(std::move(allocation));
    m_mesh.addBinding(alloc.buffer, alloc.allocation.offset);
}

void ChunkMesh::setIndexBuffer(const std::shared_ptr<VoxelEngine::Buffer>& buffer) {
    m_mesh.setIndexBuffer(buffer, vk::IndexType::Uint32, 0);
}