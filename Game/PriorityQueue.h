#pragma once
#include <vector>
#include <unordered_set>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <Engine/math.h>
#include <entt/entt.hpp>

class PriorityQueue {
public:
    struct Item {
        glm::ivec3 pos;
        entt::entity entity;
        int32_t priority;

        bool operator < (const Item& other);
    };

    PriorityQueue();

    size_t count() const { return m_set.size(); }

    void enqueue(glm::ivec3 pos, entt::entity entity);
    Item peek();
    Item dequeue();
    void remove(glm::ivec3 pos);
    void update(glm::ivec3 pos);

private:
    std::vector<Item> m_items;
    std::unordered_set<glm::ivec3> m_set;
    glm::ivec3 m_playerPos;
};