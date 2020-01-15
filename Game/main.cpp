#include <iostream>
#include <Engine/Engine.h>
#include <Engine/Window.h>
#include <Engine/Renderer.h>

#include "FrameRateCounter.h"

int main() {
    Engine engine;

    Window window(800, 600, "VoxelGame");
    engine.addWindow(window);

    Renderer renderer;
    renderer.pickPhysicalDevice(window);

    FrameRateCounter counter(0, window, "VoxelGame");
    engine.getUpdateGroup().add(counter);

    engine.run();

    return 0;
}