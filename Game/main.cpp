#include <iostream>
#include <Engine/Engine.h>
#include <Engine/RenderGraph/AcquireNode.h>
#include <Engine/RenderGraph/PresentNode.h>
#include <Engine/CameraSystem.h>

#include "FrameRateCounter.h"
#include "Renderer.h"
#include "FreeCam.h"

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
    VoxelEngine::CameraSystem cameraSystem(engine, 90);
    cameraSystem.setCamera(camera);
    engine.getUpdateGroup().add(cameraSystem);

    Renderer renderer(100, engine, cameraSystem);
    engine.getUpdateGroup().add(renderer);

    cameraSystem.setTransferNode(renderer.transferNode());

    camera.setPosition({ 0, 0, -2 });

    FreeCam freeCam(10, camera, window.input());
    engine.getUpdateGroup().add(freeCam);

    engine.run();

    renderer.wait();

    return 0;
}