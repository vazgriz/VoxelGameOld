#include <iostream>
#include <Engine/Engine.h>
#include <Engine/Window.h>

int main() {
    Engine engine;
    Window window(800, 600, "VoxelGame");

    engine.addWindow(window);
    engine.run();

    return 0;
}