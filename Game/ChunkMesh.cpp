#include "ChunkMesh.h"

ChunkMesh::ChunkMesh() {

}

void ChunkMesh::clearMesh() {
    m_mesh.setIndexCount(0);
}

bool ChunkMesh::isEmpty() const {
    return m_mesh.indexCount() == 0;
}