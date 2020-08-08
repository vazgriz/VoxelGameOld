#include <iostream>
#include <Engine/Engine.h>
#include <Engine/RenderGraph/AcquireNode.h>
#include <Engine/RenderGraph/PresentNode.h>
#include <Engine/CameraSystem.h>
#include <entt/entt.hpp>

#include "FrameRateCounter.h"
#include "Renderer.h"
#include "FreeCam.h"
#include "Chunk.h"
#include "ChunkMesh.h"
#include "ChunkUpdater.h"
#include "ChunkMesher.h"
#include "ChunkManager.h"
#include "TextureManager.h"
#include "BlockManager.h"
#include "TerrainGenerator.h"
#include "SkyboxManager.h"
#include "SelectionBox.h"
#include "MeshManager.h"

int main() {
    VoxelEngine::Engine engine;
    VoxelEngine::Window window(800, 600, "VoxelGame");
    engine.addWindow(window);

    VoxelEngine::Graphics& graphics = engine.getGraphics();
    graphics.pickPhysicalDevice(window);

    VoxelEngine::RenderGraph renderGraph(graphics.device(), 2);
    engine.setRenderGraph(renderGraph);

    FrameRateCounter counter(window, "VoxelGame");
    engine.getUpdateGroup().add(counter, 0);

    VoxelEngine::Camera camera(engine, window.getFramebufferWidth(), window.getFramebufferHeight(), glm::radians(90.0f), 0.01f, 1000.0f);
    VoxelEngine::CameraSystem cameraSystem(engine);
    cameraSystem.setCamera(camera);
    engine.getUpdateGroup().add(cameraSystem, 90);

    window.onFramebufferResized().connect<&VoxelEngine::Camera::setSize>(&camera);

    MeshManager meshManager(engine);
    SkyboxManager skyboxManager(engine, cameraSystem);
    SelectionBox selectionBox(engine, cameraSystem);
    TextureManager textureManager(engine);
    BlockManager blockManager;
    std::unique_ptr<World> worldPtr = std::make_unique<World>(blockManager);    //allocate on the heap to optimize shutdown (see below)
    World& world = *worldPtr;

    FreeCam freeCam(camera, window.input(), world, blockManager, selectionBox);
    engine.getUpdateGroup().add(freeCam, 10);

    freeCam.setPosition({ 0, 80, 0 });

    ChunkManager chunkManager(world, freeCam, 16);
    engine.getUpdateGroup().add(chunkManager, 20);

    TerrainGenerator terrainGenerator(world, chunkManager);
    terrainGenerator.run();

    ChunkUpdater chunkUpdater(engine, world, blockManager, chunkManager);
    chunkUpdater.run();

    ChunkMesher chunkMesher(engine, world, blockManager, meshManager);
    engine.getUpdateGroup().add(chunkMesher, 30);
    chunkMesher.run();

    chunkManager.setTerrainGenerator(terrainGenerator);
    chunkManager.setChunkUpdater(chunkUpdater);
    chunkManager.setChunkMesher(chunkMesher);

    Renderer renderer(engine, renderGraph, cameraSystem, world, textureManager, skyboxManager, selectionBox, meshManager);
    engine.getUpdateGroup().add(renderer, 100);

    meshManager.setTransferNode(renderer.transferNode());
    cameraSystem.setTransferNode(renderer.transferNode());
    chunkMesher.setTransferNode(renderer.transferNode());
    textureManager.createTexture(renderer.transferNode(), renderer.mipmapGenerator());
    skyboxManager.transfer(renderer.transferNode());
    skyboxManager.createPipeline(renderer.chunkRenderer().renderPass());
    selectionBox.transfer(renderer.transferNode());
    selectionBox.createMesh(renderer.transferNode(), meshManager);
    selectionBox.createPipeline(renderer.chunkRenderer().renderPass());

    engine.run();

    terrainGenerator.stop();
    chunkUpdater.stop();
    chunkMesher.stop();
    renderer.wait();

    worldPtr.release(); //drop the pointer on purpose. entt::registry deallocation is slow and not needed by this point

    return 0;
}