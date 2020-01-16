#pragma once

namespace VoxelEngine {
    class Clock {
    public:
        Clock();
        Clock(float startTime);

        void update();
        void update(float step);

        float time() const { return m_time; }
        float delta() const { return m_delta; }

    private:
        float m_time;
        float m_delta;
    };
}