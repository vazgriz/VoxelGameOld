#pragma once
#include <vector>
#include <unordered_set>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <Engine/math.h>
#include <entt/entt.hpp>
#include <algorithm>
#include <limits>

class PriorityQueue {
public:
    PriorityQueue();

    size_t count() const { return m_set.size(); }

    void enqueue(glm::ivec3 pos);
    glm::ivec3 peek();
    glm::ivec3 dequeue();
    void remove(glm::ivec3 pos);
    void update(glm::ivec3 pos);

private:
    struct Item {
        glm::ivec3 pos;
        int32_t priority;

        bool operator < (const Item& other) {
            return priority > other.priority;
        }
    };

    std::vector<Item> m_items;
    std::unordered_set<glm::ivec3> m_set;
    glm::ivec3 m_playerPos;
};