#include "Chunk.h"

bool Light::operator > (Light& other) {
    return sun > other.sun;
}

bool Light::operator < (Light& other) {
    return sun < other.sun;
}

bool Light::operator == (Light& other) {
    return sun == other.sun;
}

void Light::overwrite(Light& other) {
    if (other.sun > sun) {
        sun = other.sun;
    }
}

Chunk::PositionIterator::PositionIterator() {
    state = {};
}

Chunk::PositionIterator::PositionIterator(const Chunk::PositionIterator& other) {
    state = other.state;
}

Chunk::PositionIterator& Chunk::PositionIterator::operator=(const Chunk::PositionIterator& other) {
    state = other.state;
    return *this;
}

bool Chunk::PositionIterator::operator==(const Chunk::PositionIterator& other) const {
    return state == other.state;
}

bool Chunk::PositionIterator::operator!=(const Chunk::PositionIterator& other) const {
    return state != other.state;
}

Chunk::PositionIterator& Chunk::PositionIterator::operator++() {
    int32_t carry = increment(state.y, increment(state.x, 1));
    state.z += carry;
    return *this;
}

Chunk::PositionIterator::reference Chunk::PositionIterator::operator*() const {
    return state;
}

Chunk::PositionIterator::pointer Chunk::PositionIterator::operator->() const {
    return &state;
}

int32_t Chunk::PositionIterator::increment(int32_t& dest, int32_t value) {
    int32_t temp = dest + value;
    dest = temp & mask;
    return temp >> shiftAmount;
}

Chunk::PositionIterator::PositionIterator(glm::ivec3 state) {
    this->state = state;
}

Chunk::Chunk(entt::entity entity, glm::ivec3 pos) : m_neighbors() {
    m_worldChunkPosition = pos;
    m_loadState = ChunkLoadState::Loading;

    m_neighbors[1][1][1] = entity;

    m_lightUpdates = std::make_unique<VoxelEngine::BufferedQueue<LightUpdate>>();
}

entt::entity Chunk::neighbor(glm::ivec3 offset) {
    return m_neighbors[offset.x + 1][offset.y + 1][offset.z + 1];
}

void Chunk::setNeighbor(glm::ivec3 offset, entt::entity chunk) {
    if (offset == glm::ivec3(0, 0, 0)) {
        return;
    }

    m_neighbors[offset.x + 1][offset.y + 1][offset.z + 1] = chunk;
}

size_t Chunk::index(glm::ivec3 pos) {
    return ChunkData<Block, chunkSize>::index(pos);
}

glm::ivec3 Chunk::position(size_t index) {
    glm::ivec3 result = {};
    result.x = static_cast<int32_t>(index & mask);
    index = index >> shiftAmount;
    result.y = static_cast<int32_t>(index & mask);
    index = index >> shiftAmount;
    result.z = static_cast<int32_t>(index & mask);

    return result;
}

std::array<int32_t, 2> Chunk::divide(int32_t dividend, int32_t divisor) {
    //https://stackoverflow.com/a/39308162/8733481
    auto const divT = std::div(dividend, divisor);
    auto const I = divT.rem >= 0 ? 0 : (divisor > 0 ? 1 : -1);
    auto const qE = divT.quot - I;
    auto const rE = divT.rem + I * divisor;

    return { qE, rE };
}

std::array<glm::ivec3, 2> Chunk::divide(glm::ivec3 dividend, glm::ivec3 divisor) {
    std::array<std::array<int32_t, 2>, 3> results;

    for (size_t i = 0; i < 3; i++) {
        results[i] = divide(dividend[i], divisor[i]);
    }

    return {
        glm::ivec3(results[0][0], results[1][0], results[2][0]),
        glm::ivec3(results[0][1], results[1][1], results[2][1])
    };
}

glm::ivec3 Chunk::worldToWorldChunk(glm::ivec3 worldPos) {
    return split(worldPos)[0];
}

glm::ivec3 Chunk::worldToChunk(glm::ivec3 worldPos) {
    return split(worldPos)[1];
}

glm::ivec3 Chunk::chunkToWorld(glm::ivec3 chunkPos, glm::ivec3 worldChunkPos) {
    return chunkPos + (worldChunkPos * chunkSize);
}

std::array<glm::ivec3, 2> Chunk::split(glm::ivec3 worldPos) {
    return divide(worldPos, { Chunk::chunkSize, Chunk::chunkSize, Chunk::chunkSize });
}

const std::array<glm::ivec3, 6> Chunk::Neighbors6 = {
    glm::ivec3(1, 0, 0),    //right
    glm::ivec3(-1, 0, 0),   //left
    glm::ivec3(0, 1, 0),    //top
    glm::ivec3(0, -1, 0),   //bottom
    glm::ivec3(0, 0, 1),    //front
    glm::ivec3(0, 0, -1)    //back
};

const std::array<glm::ivec3, 4> Chunk::Neighbors4 = {
        glm::ivec3(1, 0, 0),    //right
        glm::ivec3(-1, 0, 0),   //left
        glm::ivec3(0, 0, 1),    //front
        glm::ivec3(0, 0, -1)    //back
};

const std::array<glm::ivec3, 8> Chunk::Neighbors8 = {
        glm::ivec3(1, 0, 0),
        glm::ivec3(1, 0, 1),
        glm::ivec3(0, 0, 1),
        glm::ivec3(-1, 0, 1),
        glm::ivec3(-1, 0, 0),
        glm::ivec3(-1, 0, -1),
        glm::ivec3(0, 0, -1),
        glm::ivec3(1, 0, -1),
};

const std::array<glm::ivec2, 8> Chunk::Neighbors8_2D = {
        glm::ivec2(1, 0),
        glm::ivec2(1, 1),
        glm::ivec2(0, 1),
        glm::ivec2(-1, 1),
        glm::ivec2(-1, 0),
        glm::ivec2(-1, -1),
        glm::ivec2(0, -1),
        glm::ivec2(1, -1),
};

const std::array<glm::ivec3, 26> Chunk::Neighbors26 = {
        glm::ivec3(1, 1, 0),    //upper layer
        glm::ivec3(1, 1, 1),
        glm::ivec3(0, 1, 1),
        glm::ivec3(-1, 1, 1),
        glm::ivec3(-1, 1, 0),
        glm::ivec3(-1, 1, -1),
        glm::ivec3(0, 1, -1),
        glm::ivec3(1, 1, -1),
        glm::ivec3(0, 1, 0),
        glm::ivec3(1, 0, 0),    //same layer
        glm::ivec3(1, 0, 1),
        glm::ivec3(0, 0, 1),
        glm::ivec3(-1, 0, 1),
        glm::ivec3(-1, 0, 0),
        glm::ivec3(-1, 0, -1),
        glm::ivec3(0, 0, -1),
        glm::ivec3(1, 0, -1),
        glm::ivec3(1, -1, 0),    //lower layer
        glm::ivec3(1, -1, 1),
        glm::ivec3(0, -1, 1),
        glm::ivec3(-1, -1, 1),
        glm::ivec3(-1, -1, 0),
        glm::ivec3(-1, -1, -1),
        glm::ivec3(0, -1, -1),
        glm::ivec3(1, -1, -1),
        glm::ivec3(0, -1, 0),
};

const std::array<Chunk::FaceData, 6> Chunk::NeighborFaces = {
    //right
    FaceData {
        {
            glm::ivec3(1, 1, 1),
            glm::ivec3(1, 1, 0),
            glm::ivec3(1, 0, 1),
            glm::ivec3(1, 0, 0),
        },
        {
            AmbientOcclusionOffsets {
                glm::ivec3(1, 1, 1),
                glm::ivec3(1, 1, 0),
                glm::ivec3(1, 0, 1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(1, 1, -1),
                glm::ivec3(1, 1, 0),
                glm::ivec3(1, 0, -1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(1, -1, 1),
                glm::ivec3(1, -1, 0),
                glm::ivec3(1, 0, 1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(1, -1, -1),
                glm::ivec3(1, -1, 0),
                glm::ivec3(1, 0, -1)
            },
        }
    },
    //left
    FaceData {
        {
            glm::ivec3(0, 1, 0),
            glm::ivec3(0, 1, 1),
            glm::ivec3(0, 0, 0),
            glm::ivec3(0, 0, 1),
        },
        {
            AmbientOcclusionOffsets {
                glm::ivec3(-1, 1, -1),
                glm::ivec3(-1, 1, 0),
                glm::ivec3(-1, 0, -1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(-1, 1, 1),
                glm::ivec3(-1, 1, 0),
                glm::ivec3(-1, 0, 1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(-1, -1, -1),
                glm::ivec3(-1, -1, 0),
                glm::ivec3(-1, 0, -1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(-1, -1, 1),
                glm::ivec3(-1, -1, 0),
                glm::ivec3(-1, 0, 1)
            },
        }
    },
    //top
    FaceData {
        {
            glm::ivec3(0, 1, 0),
            glm::ivec3(1, 1, 0),
            glm::ivec3(0, 1, 1),
            glm::ivec3(1, 1, 1),
        },
        {
            AmbientOcclusionOffsets {
                glm::ivec3(-1, 1, -1),
                glm::ivec3(0, 1, -1),
                glm::ivec3(-1, 1, 0)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(1, 1, -1),
                glm::ivec3(0, 1, -1),
                glm::ivec3(1, 1, 0)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(-1, 1, 1),
                glm::ivec3(0, 1, 1),
                glm::ivec3(-1, 1, 0)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(1, 1, 1),
                glm::ivec3(0, 1, 1),
                glm::ivec3(1, 1, 0)
            },
        }
    },
    //bottom
    FaceData {
        {
            glm::ivec3(1, 0, 0),
            glm::ivec3(0, 0, 0),
            glm::ivec3(1, 0, 1),
            glm::ivec3(0, 0, 1),
        },
        {
            AmbientOcclusionOffsets {
                glm::ivec3(1, -1, -1),
                glm::ivec3(1, -1, 0),
                glm::ivec3(0, -1, -1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(-1, -1, -1),
                glm::ivec3(-1, -1, 0),
                glm::ivec3(0, -1, -1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(1, -1, 1),
                glm::ivec3(1, -1, 0),
                glm::ivec3(0, -1, 1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(-1, -1, 1),
                glm::ivec3(-1, -1, 0),
                glm::ivec3(0, -1, 1)
            },
        }
    },
    //front
    FaceData {
        {
            glm::ivec3(0, 1, 1),
            glm::ivec3(1, 1, 1),
            glm::ivec3(0, 0, 1),
            glm::ivec3(1, 0, 1),
        },
        {
            AmbientOcclusionOffsets {
                glm::ivec3(-1, 1, 1),
                glm::ivec3(0, 1, 1),
                glm::ivec3(-1, 0, 1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(1, 1, 1),
                glm::ivec3(0, 1, 1),
                glm::ivec3(1, 0, 1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(-1, -1, 1),
                glm::ivec3(0, -1, 1),
                glm::ivec3(-1, 0, 1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(1, -1, 1),
                glm::ivec3(0, -1, 1),
                glm::ivec3(1, 0, 1)
            },
        }
    },
    //back
    FaceData {
        {
            glm::ivec3(1, 1, 0),
            glm::ivec3(0, 1, 0),
            glm::ivec3(1, 0, 0),
            glm::ivec3(0, 0, 0),
        },
        {
            AmbientOcclusionOffsets {
                glm::ivec3(1, 1, -1),
                glm::ivec3(0, 1, -1),
                glm::ivec3(1, 0, -1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(-1, 1, -1),
                glm::ivec3(0, 1, -1),
                glm::ivec3(-1, 0, -1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(1, -1, -1),
                glm::ivec3(0, -1, -1),
                glm::ivec3(1, 0, -1)
            },
            AmbientOcclusionOffsets {
                glm::ivec3(-1, -1, -1),
                glm::ivec3(0, -1, -1),
                glm::ivec3(-1, 0, -1)
            },
        }
    },
};

const std::array<glm::ivec2, 4> Chunk::uvFaces = {
        glm::ivec2(0, 0),
        glm::ivec2(1, 0),
        glm::ivec2(0, 1),
        glm::ivec2(1, 1)
};