#pragma once
#include <Engine/Engine.h>
#include <memory>

#include "MeshManager.h"

class ChunkMesh {
public:
    ChunkMesh();
    ChunkMesh(const ChunkMesh& other) = delete;
    ChunkMesh& operator = (const ChunkMesh& other) = delete;
    ChunkMesh(ChunkMesh&& other) = default;
    ChunkMesh& operator = (ChunkMesh&& other) = default;

    const VoxelEngine::Mesh& mesh() const { return m_mesh; }
    VoxelEngine::Mesh& mesh() { return m_mesh; }

    bool dirty() const { return m_dirty; }
    void setDirty() { m_dirty = true; }
    void clearDirty() { m_dirty = false; }

    void clearMesh();
    bool isEmpty() const;

    int32_t vertexOffset() const { return m_mesh.vertexOffset(); }
    void setVertexOffset(int32_t vertexOffset) { m_mesh.setVertexOffset(vertexOffset); }

    void clearBindings();
    void addBinding(MeshAllocation&& allocation);
    void setIndexBuffer(const std::shared_ptr<VoxelEngine::Buffer>& buffer);

    MeshAllocation& getBinding(size_t index) { return m_allocations[index]; }

private:
    VoxelEngine::Mesh m_mesh;
    bool m_dirty = false;

    std::vector<MeshAllocation> m_allocations;
};