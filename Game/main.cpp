#include <iostream>
#include <Engine/Engine.h>
#include <Engine/RenderGraph/AcquireNode.h>
#include <Engine/RenderGraph/PresentNode.h>

#include "FrameRateCounter.h"
#include "Renderer.h"

int main() {
    VoxelEngine::Engine engine;

    VoxelEngine::Window window(800, 600, "VoxelGame");
    engine.addWindow(window);

    VoxelEngine::Camera camera(window.getWidth(), window.getHeight(), glm::radians(90.0f), 0.01f, 1000.0f);

    engine.setGraphics({});
    VoxelEngine::Graphics& graphics = engine.getGraphics();
    graphics.pickPhysicalDevice(window);

    FrameRateCounter counter(0, window, "VoxelGame");
    engine.getUpdateGroup().add(counter);

    Renderer renderer(100, engine);
    engine.getUpdateGroup().add(renderer);

    engine.run();

    renderer.wait();

    return 0;
}