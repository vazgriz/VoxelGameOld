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

int main() {
    VoxelEngine::Engine engine;
    VoxelEngine::Window window(800, 600, "VoxelGame");
    engine.addWindow(window);

    VoxelEngine::Graphics& graphics = engine.getGraphics();
    graphics.pickPhysicalDevice(window);

    FrameRateCounter counter(0, window, "VoxelGame");
    engine.getUpdateGroup().add(counter);

    VoxelEngine::Camera camera(engine, window.getFramebufferWidth(), window.getFramebufferHeight(), glm::radians(90.0f), 0.01f, 1000.0f);
    VoxelEngine::CameraSystem cameraSystem(engine, 90);
    cameraSystem.setCamera(camera);
    engine.getUpdateGroup().add(cameraSystem);

    window.onFramebufferResized().connect<&VoxelEngine::Camera::setSize>(&camera);

    FreeCam freeCam(10, camera, window.input());
    engine.getUpdateGroup().add(freeCam);

    freeCam.setPosition({ -4, 20, -4 });

    entt::registry registry;
    entt::entity chunkEntity = registry.create();

    registry.assign<Chunk>(chunkEntity, glm::ivec3{});
    registry.assign<ChunkMesh>(chunkEntity);

    auto view = registry.view<Chunk, ChunkMesh>();

    ChunkUpdater chunkUpdater(20, engine, registry);
    engine.getUpdateGroup().add(chunkUpdater);

    Renderer renderer(100, engine, cameraSystem, registry);
    engine.getUpdateGroup().add(renderer);

    cameraSystem.setTransferNode(renderer.transferNode());
    chunkUpdater.setTransferNode(renderer.transferNode());

    engine.run();

    renderer.wait();

    return 0;
}