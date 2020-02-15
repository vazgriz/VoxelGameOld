#include "ChunkMesh.h"

ChunkMesh::ChunkMesh() {

}

void ChunkMesh::clearMesh() {
    m_mesh.setIndexCount(0);
    m_mesh.setIndexBuffer(nullptr, vk::IndexType::Uint32, 0);
}