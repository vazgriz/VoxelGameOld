#include "ChunkUpdater.h"
#include "Chunk.h"
#include "ChunkMesh.h"

ChunkUpdater::ChunkUpdater(VoxelEngine::Engine& engine, World& world, BlockManager& blockManager) : m_requestQueue(queueSize) {
    m_engine = &engine;
    m_world = &world;
    m_blockManager = &blockManager;

    createIndexBuffer();
}

void ChunkUpdater::setTransferNode(VoxelEngine::TransferNode& transferNode) {
    m_transferNode = &transferNode;

    m_transferNode->transfer(*m_indexBuffer, m_indexBufferSize, 0, m_indexData.data());
    m_indexData = {};
}

void ChunkUpdater::update(VoxelEngine::Clock& clock) {
    std::queue<MeshUpdate2>& queue = m_resultQueue.swapDequeue();
    auto view = m_world->registry().view<ChunkMesh>();

    while (queue.size() > 0) {
        auto& update = queue.front();
        auto entity = m_world->getEntity(update.coord);
        if (entity != entt::null) {
            transferMesh(view.get(entity), update.index);
        }
        queue.pop();
    }
}

void ChunkUpdater::run() {
    m_running = true;
    m_thread = std::thread([this] { loop(); });
}

void ChunkUpdater::stop() {
    m_running = false;
    m_requestQueue.cancel();
    m_thread.join();
}

bool ChunkUpdater::queue(glm::ivec3 coord) {
    return m_requestQueue.tryEnqueue(coord);
}

void ChunkUpdater::loop() {
    while (m_running) {
        glm::ivec3 coord;
        bool valid = m_requestQueue.dequeue(coord);
        if (!valid) return;

        update(coord);
    }
}

void ChunkUpdater::update(glm::ivec3 worldChunkPos) {
    ChunkBuffer blocks;
    LightBuffer light;
    ChunkData<Chunk*, 3> neighborChunks;
    const glm::ivec3 root = { 1, 1, 1 };
    std::queue<LightUpdate> queue;

    {
        auto lock = m_world->getReadLock();
        entt::entity entity = m_world->getEntity(worldChunkPos);
        if (entity == entt::null) return;

        auto view = m_world->registry().view<Chunk, ChunkMesh>();
        Chunk& chunk = view.get<Chunk>(entity);
        ChunkMesh& chunkMesh = view.get<ChunkMesh>(entity);

        for (auto offset : Chunk::Neighbors26) {
            entt::entity neighborEntity = chunk.neighbor(offset);

            if (m_world->valid(neighborEntity)) {
                auto& neighbor = view.get<Chunk>(neighborEntity);
                neighborChunks[root + offset] = &neighbor;
            } else {
                neighborChunks[root + offset] = nullptr;
            }
        }

        for (int32_t x = -1; x < Chunk::chunkSize + 1; x++) {
            for (int32_t y = -1; y < Chunk::chunkSize + 1; y++) {
                for (int32_t z = -1; z < Chunk::chunkSize + 1; z++) {
                    glm::ivec3 pos = { x, y, z };
                    auto results = Chunk::split(pos);
                    glm::ivec3 chunkOffset = results[0];
                    glm::ivec3 posMod = results[1];

                    if (chunkOffset == glm::ivec3()) {
                        blocks[root + pos] = chunk.blocks()[posMod];
                        light[root + pos] = chunk.light()[posMod];
                    } else {
                        auto neighborChunk = neighborChunks[root + chunkOffset];

                        if (neighborChunk != nullptr) {
                            blocks[root + pos] = neighborChunk->blocks()[posMod];
                            light[root + pos] = neighborChunk->light()[posMod];
                        } else {
                            blocks[root + pos] = Block(0);
                            light[root + pos] = Light(15);
                        }
                    }
                }
            }
        }

        auto& lightUpdates = chunk.getLightUpdates().swapDequeue();

        while (lightUpdates.size() > 0) {
            auto update = lightUpdates.front();
            lightUpdates.pop();
            queue.push(update);
        }
    }

    updateLight(queue, blocks, light, neighborChunks);

    size_t updateIndex = makeMesh(worldChunkPos, blocks, light);
    m_resultQueue.enqueue({ updateIndex, worldChunkPos });
}

void ChunkUpdater::updateLight(std::queue<LightUpdate>& queue, ChunkBuffer& chunkBuffer, LightBuffer& lightBuffer, ChunkData<Chunk*, 3>& neighborChunks) {
    const glm::ivec3 root = { 1, 1, 1 };

    while (queue.size() > 0) {
        auto light = queue.front().light;
        auto pos = queue.front().inChunkPos;
        queue.pop();

        if (!(lightBuffer[root + pos] < light)) continue;

        lightBuffer[root + pos].overwrite(light);

        for (auto offset : Chunk::Neighbors6) {
            glm::ivec3 neighborPos = pos + offset;
            auto neighborResults = Chunk::split(neighborPos);
            glm::ivec3 neighborChunkOffset = neighborResults[0];
            glm::ivec3 neighborPosMod = neighborResults[1];

            int32_t loss = 1;

            if (offset == glm::ivec3(0, 1, 0) || offset == glm::ivec3(0, -1, 0)) {
                loss = 0;
            }

            Light newLight(std::max<int8_t>(0, light.sun - loss));

            if (neighborChunkOffset == glm::ivec3()) {
                Block& block = chunkBuffer[root + neighborPosMod];

                if (block.type > 1) continue;

                Light& currentLight = lightBuffer[root + neighborPosMod];
                if (newLight > currentLight) {
                    queue.push({ newLight, neighborPosMod });
                }
            } else {
                //Chunk* neighborChunk = neighborChunks[root + neighborChunkOffset];
                //
                //if (neighborChunk != nullptr) {
                //    neighborChunk->getLightUpdates().enqueue({ newLight, neighborPosMod });
                //    m_world->update(neighborChunk->worldChunkPosition());
                //}
            }
        }
    }
}

void ChunkUpdater::createIndexBuffer() {
    uint32_t index = 0;

    for (uint32_t i = 0; i < 2048 * 6; i++) {
        m_indexData.push_back(index + 0);
        m_indexData.push_back(index + 1);
        m_indexData.push_back(index + 2);
        m_indexData.push_back(index + 1);
        m_indexData.push_back(index + 3);
        m_indexData.push_back(index + 2);
        index += 4;
    }

    m_indexCount = static_cast<uint32_t>(m_indexData.size());
    m_indexBufferSize = m_indexData.size() * sizeof(uint32_t);

    vk::BufferCreateInfo info = {};
    info.size = m_indexBufferSize;
    info.usage = vk::BufferUsageFlags::IndexBuffer | vk::BufferUsageFlags::TransferDst;
    info.sharingMode = vk::SharingMode::Exclusive;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    m_indexBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, info, allocInfo);
}

size_t ChunkUpdater::makeMesh(glm::ivec3 worldChunkPos, ChunkBuffer& chunkBuffer, LightBuffer& lightBuffer) {
    size_t index = m_updateIndex;
    m_updateIndex = (m_updateIndex + 1) % (queueSize * 2);

    MeshUpdate& update = m_updates[index];
    update.vertexData.clear();
    update.colorData.clear();
    update.uvData.clear();

    uint32_t indexCount = 0;
    const glm::ivec3 root = { 1, 1, 1 };

    auto view = m_world->registry().view<Chunk>();

    for (glm::ivec3 pos : Chunk::Positions()) {
        Block block = chunkBuffer[root + pos];
        if (block.type == 0) continue;
        if (block.type == 1) continue;
        BlockType& blockType = m_blockManager->getType(block.type);

        ChunkData<Block, 3> neighborBlocks;
        ChunkData<Light, 3> neighborLight;

        for (auto offset : Chunk::Neighbors26) {
            neighborBlocks[root + offset] = chunkBuffer[root + pos + offset];
            neighborLight[root + offset] = lightBuffer[root + pos + offset];
        }

        for (size_t i = 0; i < Chunk::Neighbors6.size(); i++) {
            glm::ivec3 offset = Chunk::Neighbors6[i];
            glm::ivec3 neighborPos = pos + offset;
            glm::ivec3 worldNeighborPos = neighborPos + (worldChunkPos * Chunk::chunkSize);

            const int32_t worldHeightMin = 0;
            const int32_t worldHeightMax = World::worldHeight * Chunk::chunkSize;

            bool visible = neighborBlocks[root + offset].type == 1 || worldNeighborPos.y >= worldHeightMax || worldNeighborPos.y < worldHeightMin;

            if (visible) {
                const Chunk::FaceData& faceData = Chunk::NeighborFaces[i];
                size_t faceIndex = blockType.getFaceIndex(i);

                for (size_t j = 0; j < faceData.vertices.size(); j++) {
                    int32_t light = neighborLight[root + offset].sun;

                    for (size_t k = 0; k < 3; k++) {
                        light += neighborLight[root + faceData.ambientOcclusion[j][k]].sun;
                    }

                    light /= 4;
                    light = std::max(light * 17, 0);

                    update.vertexData.push_back(glm::i8vec4(pos + faceData.vertices[j], 0));
                    update.colorData.push_back(glm::i8vec4(light, light, light, 0));
                    update.uvData.push_back(glm::i8vec4(Chunk::uvFaces[j], static_cast<uint8_t>(faceIndex), 0));
                }

                indexCount++;
            }
        }
    }

    update.indexCount = indexCount * 6;

    return index;
}

void ChunkUpdater::transferMesh(ChunkMesh& chunkMesh, size_t index) {
    MeshUpdate& update = m_updates[index];

    if (update.indexCount == 0) {
        chunkMesh.mesh().setIndexCount(0);
        chunkMesh.mesh().setIndexBuffer(nullptr, vk::IndexType::Uint32, 0);
        return;
    }

    size_t vertexSize = update.vertexData.size() * sizeof(glm::i8vec4);
    size_t colorSize = update.colorData.size() * sizeof(glm::i8vec4);
    size_t uvSize = update.uvData.size() * sizeof(glm::i8vec4);

    if (chunkMesh.mesh().bindingCount() == 0 || chunkMesh.mesh().getBinding(0)->size() < vertexSize) {
        vk::BufferCreateInfo vertexInfo = {};
        vertexInfo.size = vertexSize;
        vertexInfo.usage = vk::BufferUsageFlags::VertexBuffer | vk::BufferUsageFlags::TransferDst;
        vertexInfo.sharingMode = vk::SharingMode::Exclusive;

        vk::BufferCreateInfo colorInfo = vertexInfo;
        colorInfo.size = colorSize;

        vk::BufferCreateInfo uvInfo = vertexInfo;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        std::shared_ptr<VoxelEngine::Buffer> vertexBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, vertexInfo, allocInfo);
        std::shared_ptr<VoxelEngine::Buffer> colorBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, colorInfo, allocInfo);
        std::shared_ptr<VoxelEngine::Buffer> uvBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, uvInfo, allocInfo);

        chunkMesh.mesh().clearBindings();
        chunkMesh.mesh().addBinding(vertexBuffer);
        chunkMesh.mesh().addBinding(colorBuffer);
        chunkMesh.mesh().addBinding(uvBuffer);

        chunkMesh.mesh().setIndexBuffer(m_indexBuffer, vk::IndexType::Uint32, 0);
        chunkMesh.mesh().setIndexCount(update.indexCount);
    }

    m_transferNode->transfer(*chunkMesh.mesh().getBinding(0), vertexSize, 0, update.vertexData.data());
    m_transferNode->transfer(*chunkMesh.mesh().getBinding(1), colorSize, 0, update.colorData.data());
    m_transferNode->transfer(*chunkMesh.mesh().getBinding(2), uvSize, 0, update.uvData.data());

    chunkMesh.setDirty();
}