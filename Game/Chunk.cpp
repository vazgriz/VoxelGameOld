#include "Chunk.h"

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

Chunk::Chunk(glm::ivec3 pos) {
    m_worldChunkPosition = pos;
}

size_t Chunk::index(glm::ivec3 pos) {
    return pos.x + (pos.y * chunkSize) + (pos.z * chunkSize * chunkSize);
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

glm::ivec2 Chunk::divide(int32_t dividend, int32_t divisor) {
    //https://stackoverflow.com/a/39308162/8733481
    auto const divT = std::div(dividend, divisor);
    auto const I = divT.rem >= 0 ? 0 : (divisor > 0 ? 1 : -1);
    auto const qE = divT.quot - I;
    auto const rE = divT.rem + I * divisor;

    return glm::ivec2(qE, rE);
}

glm::ivec3 Chunk::worldToWorldChunk(glm::ivec3 worldPos) {
    return glm::ivec3(
        divide(worldPos.x, chunkSize).x,
        divide(worldPos.y, chunkSize).x,
        divide(worldPos.z, chunkSize).x
    );
}

glm::ivec3 Chunk::worldToChunk(glm::ivec3 worldPos) {
    return glm::ivec3(
        divide(worldPos.x, chunkSize).y,
        divide(worldPos.y, chunkSize).y,
        divide(worldPos.z, chunkSize).y
    );
}

glm::ivec3 Chunk::chunkToWorld(glm::ivec3 chunkPos, glm::ivec3 worldChunkPos) {
    return chunkPos + (worldChunkPos * static_cast<int32_t>(chunkSize));
}