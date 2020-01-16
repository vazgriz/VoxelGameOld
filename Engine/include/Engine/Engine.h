#pragma once
#include "Engine/Buffer.h"
#include "Engine/Clock.h"
#include "Engine/Mesh.h"
#include "Engine/Graphics.h"
#include "Engine/MemoryManager.h"
#include "Engine/Mesh.h"
#include "Engine/RenderSystem.h"
#include "Engine/System.h"
#include "Engine/Window.h"

namespace VoxelEngine {
    class Engine {
    public:
        Engine();
        ~Engine();

        Graphics& getGraphics() { return *m_graphics; }
        SystemGroup& getUpdateGroup() { return *m_updateGroup; }

        void run();

        void setGraphics(Graphics&& graphics);
        void addWindow(Window& window);

    private:
        Window* m_window;
        std::unique_ptr<SystemGroup> m_updateGroup;
        std::unique_ptr<Clock> m_updateClock;
        std::unique_ptr<Graphics> m_graphics;
    };
}