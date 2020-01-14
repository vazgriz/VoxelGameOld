#pragma once
#include <Engine/System.h>
#include <Engine/Window.h>
#include <Engine/Clock.h>

class FrameRateCounter : public System {
public:
    FrameRateCounter(int32_t priority, Window& window, std::string titlePrefix);

    void update(Clock& clock);

private:
    Window* m_window;
    std::string m_titlePrefix;
    size_t m_frameCount;
    float m_timer;
};