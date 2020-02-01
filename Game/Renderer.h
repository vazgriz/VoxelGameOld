#pragma once
#include <Engine/System.h>
#include <Engine/RenderGraph/RenderGraph.h>
#include <Engine/RenderGraph/AcquireNode.h>
#include <Engine/RenderGraph/PresentNode.h>
#include <Engine/RenderGraph/TransferNode.h>
#include <Engine/CameraSystem.h>
#include <entt/signal/sigh.hpp>
#include "Chunk.h"
#include "ChunkRenderer.h"
#include "TextureManager.h"
#include "MipmapGenerator.h"
#include "SkyboxManager.h"
#include "World.h"

class Renderer : public VoxelEngine::System {
public:
    Renderer(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& renderGraph, VoxelEngine::CameraSystem& cameraSystem, World& world, TextureManager& textureManager, SkyboxManager& skyboxManager);

    VoxelEngine::TransferNode& transferNode() const { return *m_transferNode; }
    MipmapGenerator& mipmapGenerator() const { return *m_mipmapGenerator; }
    ChunkRenderer& chunkRenderer() const { return *m_chunkRenderer; }

    void wait();

    void update(VoxelEngine::Clock& clock);

private:
    VoxelEngine::Engine* m_engine;
    VoxelEngine::Graphics* m_graphics;
    VoxelEngine::RenderGraph* m_renderGraph;
    VoxelEngine::AcquireNode* m_acquireNode;
    VoxelEngine::PresentNode* m_presentNode;
    VoxelEngine::TransferNode* m_transferNode;
    ChunkRenderer* m_chunkRenderer;
    MipmapGenerator* m_mipmapGenerator;
};