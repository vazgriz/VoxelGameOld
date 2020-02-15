#pragma once
#include <stdint.h>
#include <Engine/math.h>
#include <Engine/BlockingQueue.h>
#include <Engine/BufferedQueue.h>
#include <array>
#include <entt/entt.hpp>

enum class ChunkLoadState {
    Unloaded,
    Loading,
    Loaded,
    Unloading
};

enum class ChunkDirection {
    North = 0,
    NorthEast,
    East,
    SouthEast,
    South,
    SouthWest,
    West,
    NorthWest
};

struct Block {
    uint8_t type;

    Block() : type(0) {}
    Block(size_t type) : type(static_cast<uint8_t>(type)) {}
};

struct Light {
    int8_t sun;

    Light() : sun(0) {}
    Light(int32_t sun) : sun(static_cast<int8_t>(sun)) {}

    bool operator > (Light& other);
    bool operator < (Light& other);
    bool operator == (Light& other);

    void overwrite(Light& other);
};

struct LightUpdate {
    Light light;
    glm::ivec3 inChunkPos;
};

template <typename T, size_t Size>
class ChunkData {
public:
    using Array = std::array<T, Size * Size * Size>;

    ChunkData() : m_data() {

    }

    auto begin() { return m_data.begin(); }
    auto begin() const { return m_data.begin(); }
    auto end() { return m_data.end(); }
    auto end() const { return m_data.end(); }

    T& operator [] (glm::ivec3 pos) {
        return m_data[index(pos)];
    }

    static size_t index(glm::ivec3 pos) {
        return pos.x + (pos.y * Size) + (pos.z * Size * Size);
    }

    static glm::ivec3 position(size_t index) {
        glm::ivec3 result;
        result.z = index % Size;
        index /= Size;
        result.y = index % Size;
        index /= Size;
        result.x = index;

        return result;
    }

    T* data() { return m_data.data(); }
    const T* data() const { return m_data.data(); }

private:
    Array m_data;
};

class Chunk {
public:
    static const int32_t chunkSize = 16;

    struct Positions;

    struct PositionIterator {
        friend struct Positions;
        //iterate through all positions in chunk
        typedef typename std::ptrdiff_t difference_type;
        typedef typename glm::ivec3 value_type;
        typedef typename const glm::ivec3& reference;
        typedef typename const glm::ivec3* pointer;
        typedef std::forward_iterator_tag iterator_category;

        PositionIterator();
        PositionIterator(const Chunk::PositionIterator& other);
        PositionIterator& operator=(const Chunk::PositionIterator& other);

        bool operator==(const PositionIterator& other) const;
        bool operator!=(const PositionIterator& other) const;

        PositionIterator& PositionIterator::operator++();
        reference operator*() const;
        pointer operator->() const;

    private:
        glm::ivec3 state;

        int32_t increment(int32_t& dest, int32_t value);

        PositionIterator(glm::ivec3 state);
    };

    struct Positions {
        PositionIterator begin() const { return PositionIterator(); }
        PositionIterator end() const { return PositionIterator({ 0, 0, chunkSize }); }
    };

    Chunk(entt::entity entity, glm::ivec3 pos);
    Chunk(const Chunk& other) = delete;
    Chunk& operator = (const Chunk& other) = delete;
    Chunk(Chunk&& other) = default;
    Chunk& operator = (Chunk&& other) = default;

    void reset();

    glm::ivec3 worldChunkPosition() const { return m_worldChunkPosition; }
    void setWorldChunkPosition(glm::ivec3 worldChunkPos) { m_worldChunkPosition = worldChunkPos; }

    ChunkLoadState loadState() const { return m_loadState; }
    void setLoadState(ChunkLoadState loadState) { m_loadState = loadState; }

    entt::entity neighbor(glm::ivec3 offset);
    void setNeighbor(glm::ivec3 offset, entt::entity chunk);

    static size_t index(glm::ivec3 pos);
    static glm::ivec3 position(size_t index);

    static std::array<int32_t, 2> divide(int32_t dividend, int32_t divisor);
    static std::array<glm::ivec3, 2> divide(glm::ivec3 dividend, glm::ivec3 divisor);
    static glm::ivec3 worldToWorldChunk(glm::ivec3 worldPos);
    static glm::ivec3 worldToChunk(glm::ivec3 worldPos);
    static glm::ivec3 chunkToWorld(glm::ivec3 chunkPos, glm::ivec3 worldChunkPos);
    static std::array<glm::ivec3, 2> split(glm::ivec3 worldPos);

    ChunkData<Block, chunkSize>& blocks() { return *m_blocks; }
    const ChunkData<Block, chunkSize>& blocks() const { return *m_blocks; }

    ChunkData<Light, chunkSize>& light() { return *m_light; }
    const ChunkData<Light, chunkSize>& light() const { return *m_light; }

    VoxelEngine::BufferedQueue<LightUpdate>& getLightUpdates() { return *m_lightUpdates; };

    static const std::array<glm::ivec3, 6> Neighbors6;
    static const std::array<glm::ivec3, 4> Neighbors4;
    static const std::array<glm::ivec3, 8> Neighbors8;
    static const std::array<glm::ivec2, 8> Neighbors8_2D;
    static const std::array<glm::ivec3, 26> Neighbors26;

    using LightOffsets = std::array<glm::ivec3, 3>;

    struct FaceData {
        std::array<glm::ivec3, 4> vertices;
        std::array<LightOffsets, 4> ambientOcclusion;
    };

    static const std::array<FaceData, 6> NeighborFaces;
    static const std::array<glm::ivec2, 4> uvFaces;

private:
    static const int32_t mask = chunkSize - 1;
    static const int32_t shiftAmount = 4;

    glm::ivec3 m_worldChunkPosition;
    std::unique_ptr<ChunkData<Block, chunkSize>> m_blocks;
    std::unique_ptr<ChunkData<Light, chunkSize>> m_light;
    ChunkLoadState m_loadState;
    std::array<std::array<std::array<entt::entity, 3>, 3>, 3 > m_neighbors;
    std::unique_ptr<VoxelEngine::BufferedQueue<LightUpdate>> m_lightUpdates;
};