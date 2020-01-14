#pragma once
#include "Engine/Window.h"

class Engine {
public:
    Engine();
    ~Engine();

    void run();

    void addWindow(Window& window);

private:
    Window* m_window;
};