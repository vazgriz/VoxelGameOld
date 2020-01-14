#pragma once
#include "Engine/Window.h"
#include "Engine/System.h"

class Engine {
public:
    Engine();
    ~Engine();

    SystemGroup& getUpdateGroup() { return *m_updateGroup; }

    void run();

    void addWindow(Window& window);

private:
    Window* m_window;
    std::unique_ptr<SystemGroup> m_updateGroup;
    std::unique_ptr<Clock> m_updateClock;
};