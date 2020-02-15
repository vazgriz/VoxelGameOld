#pragma once
#include <Engine/Engine.h>
#include <memory>

class ChunkMesh {
public:
    ChunkMesh();

    const VoxelEngine::Mesh& mesh() const { return m_mesh; }
    VoxelEngine::Mesh& mesh() { return m_mesh; }

    bool dirty() const { return m_dirty; }
    void setDirty() { m_dirty = true; }
    void clearDirty() { m_dirty = false; }

    void clearMesh();

private:
    VoxelEngine::Mesh m_mesh;
    bool m_dirty = false;
};