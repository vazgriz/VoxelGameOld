#pragma once
#include <stdint.h>
#include <Engine/math.h>
#include <array>

struct Block {
    uint8_t type;

    Block() : type(0) {}
};

template <typename T, size_t Size>
class ChunkData {
public:
    using Array = std::array<T, Size * Size * Size>;

    ChunkData() : m_data(){

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
    static const uint32_t chunkSize = 16;

    struct Positions;

    struct PositionIterator {
        friend struct Positions;
        //iterate through all positions in chunk
        typedef typename std::ptrdiff_t difference_type;
        typedef typename glm::ivec3 value_type;
        typedef typename const glm::ivec3& reference;
        typedef typename const glm::ivec3* pointer;
        typedef std::forward_iterator_tag iterator_category;

        PositionIterator() {
            state = {};
        }

        PositionIterator(const PositionIterator& other) {
            state = other.state;
        }

        PositionIterator& operator=(const PositionIterator& other) {
            state = other.state;
            return *this;
        }

        bool operator==(const PositionIterator& other) const {
            return state == other.state;
        }

        bool operator!=(const PositionIterator& other) const {
            return state != other.state;
        }

        PositionIterator& operator++() {
            int32_t carry = increment(state.y, increment(state.x, 1));
            state.z += carry;
            return *this;
        }

        reference operator*() const {
            return state;
        }

        pointer operator->() const {
            return &state;
        }

    private:
        glm::ivec3 state;

        int32_t increment(int32_t& dest, int32_t value) {
            int32_t temp = dest + value;
            dest = temp & mask;
            return temp >> shiftAmount;
        }

        PositionIterator(glm::ivec3 state) {
            this->state = state;
        }
    };

    struct Positions {
        PositionIterator begin() const { return PositionIterator(); }
        PositionIterator end() const { return PositionIterator({ 0, 0, 16 }); }
    };

    Chunk(glm::ivec3 pos);

    static size_t index(glm::ivec3 pos) {
        return pos.x + (pos.y * chunkSize) + (pos.z * chunkSize * chunkSize);
    }

    static glm::ivec3 position(size_t index) {
        glm::ivec3 result = {};
        result.x = static_cast<int32_t>(index & mask);
        index = index >> shiftAmount;
        result.y = static_cast<int32_t>(index & mask);
        index = index >> shiftAmount;
        result.z = static_cast<int32_t>(index & mask);

        return result;
    }

    ChunkData<Block, chunkSize>& blocks() { return m_blocks; }
    const ChunkData<Block, chunkSize>& blocks() const { return m_blocks; }

    Positions positions() {
        return Positions();
    }

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

private:
    static const int32_t mask = chunkSize - 1;
    static const int32_t shiftAmount = 4;
    glm::ivec3 m_chunkPosition;
    ChunkData<Block, chunkSize> m_blocks;
};