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
#include "ChunkManager.h"

int main() {
    VoxelEngine::Engine engine;
    VoxelEngine::Window window(800, 600, "VoxelGame");
    engine.addWindow(window);

    VoxelEngine::Graphics& graphics = engine.getGraphics();
    graphics.pickPhysicalDevice(window);

    FrameRateCounter counter(window, "VoxelGame");
    engine.getUpdateGroup().add(counter, 0);

    VoxelEngine::Camera camera(engine, window.getFramebufferWidth(), window.getFramebufferHeight(), glm::radians(90.0f), 0.01f, 1000.0f);
    VoxelEngine::CameraSystem cameraSystem(engine);
    cameraSystem.setCamera(camera);
    engine.getUpdateGroup().add(cameraSystem, 90);

    window.onFramebufferResized().connect<&VoxelEngine::Camera::setSize>(&camera);

    FreeCam freeCam(camera, window.input());
    engine.getUpdateGroup().add(freeCam, 10);

    freeCam.setPosition({ -4, 20, -4 });

    entt::registry registry;

    ChunkManager chunkManager(registry, freeCam);
    engine.getUpdateGroup().add(chunkManager, 20);

    ChunkUpdater chunkUpdater(engine, registry);
    engine.getUpdateGroup().add(chunkUpdater, 30);

    Renderer renderer(engine, cameraSystem, registry);
    engine.getUpdateGroup().add(renderer, 100);

    cameraSystem.setTransferNode(renderer.transferNode());
    chunkUpdater.setTransferNode(renderer.transferNode());

    engine.run();

    renderer.wait();

    return 0;
}