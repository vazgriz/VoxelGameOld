#pragma once
#include <stdint.h>
#include <Engine/math.h>
#include <array>

enum class ChunkLoadState {
    Unloaded,
    Loading,
    Loaded,
    Unloading
};

enum class ChunkActiveState {
    Inactive,
    Active
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
        return m_data[Chunk::index(pos)];
    }

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

    Chunk(glm::ivec3 pos);

    glm::ivec3 worldChunkPosition() const { return m_worldChunkPosition; }

    ChunkLoadState loadState() const { return m_loadState; }
    void setLoadState(ChunkLoadState loadState) { m_loadState = loadState; }

    ChunkActiveState activeState() const { return m_activeState; }
    void setActiveState(ChunkActiveState activeState) { m_activeState = activeState; }

    static size_t index(glm::ivec3 pos);
    static glm::ivec3 position(size_t index);

    static glm::ivec2 divide(int32_t dividend, int32_t divisor);
    static glm::ivec3 worldToWorldChunk(glm::ivec3 worldPos);
    static glm::ivec3 worldToChunk(glm::ivec3 worldPos);
    static glm::ivec3 chunkToWorld(glm::ivec3 chunkPos, glm::ivec3 worldChunkPos);

    ChunkData<Block, chunkSize>& blocks() { return m_blocks; }
    const ChunkData<Block, chunkSize>& blocks() const { return m_blocks; }

    static constexpr std::array<glm::ivec3, 6> Neighbors6 = {
        glm::ivec3(1, 0, 0),    //right
        glm::ivec3(-1, 0, 0),   //left
        glm::ivec3(0, 1, 0),    //top
        glm::ivec3(0, -1, 0),   //bottom
        glm::ivec3(0, 0, 1),    //front
        glm::ivec3(0, 0, -1)    //back
    };

    using FaceArray = const std::array<glm::ivec3, 4>;

    static constexpr std::array<FaceArray, 6> NeighborFaces = {
        //right
        FaceArray {
            glm::ivec3(1, 1, 1),
            glm::ivec3(1, 1, 0),
            glm::ivec3(1, 0, 1),
            glm::ivec3(1, 0, 0),
        },
        //left
        FaceArray {
            glm::ivec3(0, 1, 0),
            glm::ivec3(0, 1, 1),
            glm::ivec3(0, 0, 0),
            glm::ivec3(0, 0, 1),
        },
        //top
        FaceArray {
            glm::ivec3(0, 1, 0),
            glm::ivec3(1, 1, 0),
            glm::ivec3(0, 1, 1),
            glm::ivec3(1, 1, 1),
        },
        //bottom
        FaceArray {
            glm::ivec3(1, 0, 0),
            glm::ivec3(0, 0, 0),
            glm::ivec3(1, 0, 1),
            glm::ivec3(0, 0, 1),
        },
        //front
        FaceArray {
            glm::ivec3(0, 1, 1),
            glm::ivec3(1, 1, 1),
            glm::ivec3(0, 0, 1),
            glm::ivec3(1, 0, 1),
        },
        //back
        FaceArray {
            glm::ivec3(1, 1, 0),
            glm::ivec3(0, 1, 0),
            glm::ivec3(1, 0, 0),
            glm::ivec3(0, 0, 0),
        },
    };

    static constexpr std::array<glm::ivec2, 4> uvFaces = {
        glm::ivec2(0, 0),
        glm::ivec2(1, 0),
        glm::ivec2(0, 1),
        glm::ivec2(1, 1)
    };

private:
    static const int32_t mask = chunkSize - 1;
    static const int32_t shiftAmount = 4;

    glm::ivec3 m_worldChunkPosition;
    ChunkData<Block, chunkSize> m_blocks;
    ChunkLoadState m_loadState;
    ChunkActiveState m_activeState;
};