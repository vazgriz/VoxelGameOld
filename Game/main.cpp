#include <iostream>
#include <Engine/Engine.h>
#include <Engine/RenderGraph/AcquireNode.h>
#include <Engine/RenderGraph/PresentNode.h>
#include <Engine/CameraSystem.h>

#include "FrameRateCounter.h"
#include "Renderer.h"

int main() {
    VoxelEngine::Engine engine;

    VoxelEngine::Window window(800, 600, "VoxelGame");
    engine.addWindow(window);

    engine.setGraphics({});
    VoxelEngine::Graphics& graphics = engine.getGraphics();
    graphics.pickPhysicalDevice(window);

    FrameRateCounter counter(0, window, "VoxelGame");
    engine.getUpdateGroup().add(counter);

    VoxelEngine::Camera camera(engine, window.getWidth(), window.getHeight(), glm::radians(90.0f), 0.01f, 1000.0f);
    VoxelEngine::CameraSystem cameraSystem(engine, 1);
    engine.getUpdateGroup().add(cameraSystem);

    Renderer renderer(100, engine);
    engine.getUpdateGroup().add(renderer);

    cameraSystem.setTransferNode(renderer.transferNode());

    engine.run();

    renderer.wait();

    return 0;
}