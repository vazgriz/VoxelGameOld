#include <iostream>
#include <Engine/Engine.h>

#include "FrameRateCounter.h"
#include "TriangleRenderer.h"

int main() {
    VoxelEngine::Engine engine;

    VoxelEngine::Window window(800, 600, "VoxelGame");
    engine.addWindow(window);

    engine.setGraphics({});
    VoxelEngine::Graphics& graphics = engine.getGraphics();
    graphics.pickPhysicalDevice(window);

    FrameRateCounter counter(0, window, "VoxelGame");
    engine.getUpdateGroup().add(counter);

    VoxelEngine::RenderSystem renderSystem(100, graphics);
    engine.getUpdateGroup().add(renderSystem);

    TriangleRenderer triangleRenderer(10, engine, renderSystem);
    engine.getUpdateGroup().add(triangleRenderer);

    engine.run();

    renderSystem.wait();

    return 0;
}