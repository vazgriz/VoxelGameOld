#include <iostream>
#include <Engine/Engine.h>

#include "FrameRateCounter.h"
#include "TriangleRenderer.h"

int main() {
    Engine engine;

    Window window(800, 600, "VoxelGame");
    engine.addWindow(window);

    Renderer renderer;
    renderer.pickPhysicalDevice(window);

    FrameRateCounter counter(0, window, "VoxelGame");
    engine.getUpdateGroup().add(counter);

    RenderSystem renderSystem(100, renderer);
    engine.getUpdateGroup().add(renderSystem);

    TriangleRenderer triangleRenderer(10, renderer, renderSystem);
    engine.getUpdateGroup().add(triangleRenderer);

    engine.run();

    renderSystem.wait();

    return 0;
}