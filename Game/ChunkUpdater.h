#pragma once
#include <Engine/Engine.h>
#include <Engine/RenderGraph/TransferNode.h>
#include <entt/entt.hpp>
#include "Chunk.h"
#include "ChunkMesh.h"

class ChunkUpdaterNode;

class ChunkUpdater : public VoxelEngine::System {
public:
    ChunkUpdater(VoxelEngine::Engine& engine, entt::registry& registry);

    void setTransferNode(VoxelEngine::TransferNode& transferNode);

    void update(VoxelEngine::Clock& clock);

private:
    VoxelEngine::Engine* m_engine;
    VoxelEngine::TransferNode* m_transferNode;
    entt::registry* m_registry;
    std::vector<glm::ivec3> m_vertexData;
    std::vector<glm::i8vec4> m_colorData;
    std::vector<glm::ivec3> m_uvData;
    std::vector<uint32_t> m_indexData;

    void makeMesh(Chunk& chunk, ChunkMesh& chunkMesh);
    void transferMesh(ChunkMesh& chunkMesh);
};