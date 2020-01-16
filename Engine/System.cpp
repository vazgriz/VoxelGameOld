#include <Engine/System.h>
#include <algorithm>

using namespace VoxelEngine;

System::System(int32_t priority) {
    m_group = nullptr;
    setPriority(priority);
}

void System::setPriority(int32_t priority) {
    m_priority = priority;
    if (m_group != nullptr) {
        m_group->setDirty();
    }
}

int32_t System::getPriority() const {
    return m_priority;
}

SystemGroup::SystemGroup(Clock& clock) {
    m_clock = &clock;
}

void SystemGroup::add(System& system) {
    m_systems.push_back(&system);
    setDirty();
}

void SystemGroup::remove(System& system) {
    m_systems.erase(std::remove(m_systems.begin(), m_systems.end(), &system), m_systems.end());
}

void SystemGroup::update() {
    if (m_dirty) {
        std::sort(m_systems.begin(), m_systems.end(), [](System* a, System* b) {
            return a->getPriority() < b->getPriority();
        });
    }

    for (auto system : m_systems) {
        system->preUpdate(*m_clock);
    }

    for (auto system : m_systems) {
        system->update(*m_clock);
    }
}

void SystemGroup::setDirty() {
    m_dirty = true;
}