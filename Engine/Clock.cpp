#include "Engine/Clock.h"
#include <GLFW/glfw3.h>

Clock::Clock() {
    m_time = static_cast<float>(glfwGetTime());
    m_delta = 0;
}

Clock::Clock(float startTime) {
    m_time = startTime;
    m_delta = 0;
}

void Clock::update() {
    float now = static_cast<float>(glfwGetTime());
    m_delta = now - m_time;
    m_time = now;
}

void Clock::update(float step) {
    m_delta = step;
    m_time += step;
}