#pragma once
#include <entt/entt.hpp>
#include <Engine/System.h>
#include <Engine/BufferedQueue.h>
#include <unordered_map>
#include <queue>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "FreeCam.h"
#include "Chunk.h"
#include "World.h"
#include "PriorityQueue.h"

class TerrainGenerator;
struct TerrainResults;
class ChunkUpdater;
struct UpdateResults;
class ChunkMesher;

class ChunkGroup {
public:
    ChunkGroup(glm::ivec2 coord, World& world);
    ChunkGroup(const ChunkGroup& other) = delete;
    ChunkGroup& operator = (const ChunkGroup& other) = delete;
    ChunkGroup(ChunkGroup&& other) = default;
    ChunkGroup& operator = (ChunkGroup&& other) = default;
    ~ChunkGroup();

    const std::vector<entt::entity>& chunks() const { return m_chunks; }

    ChunkLoadState loadState() const { return m_loadState; }
    void setLoadState(ChunkLoadState loadState);

    ChunkGroup* getNeighbor(ChunkDirection dir);
    ChunkGroup* getNeighbor(glm::ivec2 offset);
    ChunkGroup* getNeighbor(glm::ivec3 offset);
    void setNeighbor(ChunkDirection dir, ChunkGroup* group);

    static ChunkDirection getOpposite(ChunkDirection dir);
    static ChunkDirection getDirection(glm::ivec2 offset);

private:
    ChunkLoadState m_loadState;
    glm::ivec2 m_coord;
    World* m_world;
    std::vector<entt::entity> m_chunks;

    std::array<ChunkGroup*, 8> m_neighbors;
};

class ChunkManager : public VoxelEngine::System {
public:
    static const int32_t worldHeight = 16;

    ChunkManager(World& world, FreeCam& freeCam, int32_t viewDistance);

    void setTerrainGenerator(TerrainGenerator& terrainGenerator);
    void setChunkUpdater(ChunkUpdater& chunkUpdater);
    void setChunkMesher(ChunkMesher& chunkMesher);

    VoxelEngine::BufferedQueue<TerrainResults>& generateResultQueue() { return m_generateResultQueue; }
    VoxelEngine::BufferedQueue<UpdateResults>& updateResultQueue() { return m_updateResultQueue; }

    void update(VoxelEngine::Clock& clock);

private:
    using ChunkMap = std::unordered_map<glm::ivec2, ChunkGroup>;
    World* m_world;
    FreeCam* m_freeCam;
    TerrainGenerator* m_terrainGenerator;
    ChunkUpdater* m_chunkUpdater;
    ChunkMesher* m_chunkMesher;
    glm::ivec3 m_lastPos;
    ChunkMap m_chunkMap;
    int32_t m_viewDistance;
    int32_t m_viewDistance2;

    PriorityQueue m_generateQueue;
    VoxelEngine::BufferedQueue<TerrainResults> m_generateResultQueue;
    PriorityQueue m_updateQueue;
    VoxelEngine::BufferedQueue<UpdateResults> m_updateResultQueue;
    std::queue<glm::ivec3> m_updateRequeue;
    PriorityQueue m_meshingQueue;
    std::queue<glm::ivec3> m_meshingRequeue;

    ChunkGroup& makeChunkGroup(glm::ivec2 coord);
    ChunkMap::iterator destroyChunkGroup(ChunkMap::iterator it, glm::ivec2 coord);
    static int32_t distance2(glm::ivec2 a, glm::ivec2 b);
};