#pragma once
#include <Engine/System.h>
#include <Engine/Window.h>
#include <Engine/Clock.h>

class FrameRateCounter : public VoxelEngine::System {
public:
    FrameRateCounter(VoxelEngine::Window& window, std::string titlePrefix);

    void update(VoxelEngine::Clock& clock);

private:
    VoxelEngine::Window* m_window;
    std::string m_titlePrefix;
    size_t m_frameCount;
    float m_timer;
};