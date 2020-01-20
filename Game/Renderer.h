#pragma once
#include <Engine/System.h>
#include <Engine/RenderGraph/RenderGraph.h>
#include <Engine/RenderGraph/AcquireNode.h>
#include <Engine/RenderGraph/PresentNode.h>
#include <Engine/RenderGraph/TransferNode.h>
#include <Engine/CameraSystem.h>
#include "Chunk.h"
#include "ChunkRenderer.h"

class Renderer : public VoxelEngine::System {
public:
    Renderer(uint32_t priority, VoxelEngine::Engine& engine, VoxelEngine::CameraSystem& cameraSystem, Chunk& chunk);

    VoxelEngine::TransferNode& transferNode() const { return *m_transferNode; }

    void wait();

    void update(VoxelEngine::Clock& clock);

private:
    VoxelEngine::Engine* m_engine;
    VoxelEngine::Graphics* m_graphics;
    std::unique_ptr<VoxelEngine::RenderGraph> m_renderGraph;
    VoxelEngine::AcquireNode* m_acquireNode;
    VoxelEngine::PresentNode* m_presentNode;
    VoxelEngine::TransferNode* m_transferNode;
    ChunkRenderer* m_chunkRenderer;
};