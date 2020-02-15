cmake_minimum_required (VERSION 3.8)

project ("Game")

add_library("FastNoise"
    ${FASTNOISE_ROOT}/FastNoise.h
    ${FASTNOISE_ROOT}/FastNoise.cpp
)

add_executable("Game"
    stb.cpp
    main.cpp
    FrameRateCounter.h
    FrameRateCounter.cpp
    Renderer.h
    Renderer.cpp
    FreeCam.h
    FreeCam.cpp
    Chunk.h
    Chunk.cpp
    ChunkRenderer.h
    ChunkRenderer.cpp
    ChunkMesh.h
    ChunkMesh.cpp
    ChunkUpdater.h
    ChunkUpdater.cpp
    ChunkMesher.h
    ChunkMesher.cpp
    ChunkManager.h
    ChunkManager.cpp
    TextureManager.h
    TextureManager.cpp
    MipmapGenerator.h
    MipmapGenerator.cpp
    BlockManager.h
    BlockManager.cpp
    World.h
    World.cpp
    PriorityQueue.h
    PriorityQueue.cpp
    TerrainGenerator.h
    TerrainGenerator.cpp
    SkyboxManager.h
    SkyboxManager.cpp
)

target_link_libraries("Game"
    "Engine"
    "FastNoise"
)

target_include_directories("Game"
    PRIVATE ${STB_INCLUDE}
    PRIVATE ${FASTNOISE_ROOT}
)

set(SHADER_SOURCES
    "shaders/shader.vert" ;
    "shaders/shader.frag" ;
    "shaders/skybox.vert" ;
    "shaders/skybox.frag" ;
)
set(SPIRV_BINARY_FILES)

foreach(SHADER_SOURCE ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER_SOURCE} NAME)
    set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_COMPILER} "${PROJECT_SOURCE_DIR}/${SHADER_SOURCE}" -o ${SPIRV}
        DEPENDS ${SHADER_SOURCE})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(SHADER_SOURCE)

add_custom_target(
    CompileShaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)

set(TEXTURES
    "resources/dirt.png";
    "resources/grass_side.png";
    "resources/grass_top.png";
    "resources/stone.png";
    "resources/sky_up.png";
    "resources/sky_down.png";
    "resources/sky_right.png";
    "resources/sky_left.png";
    "resources/sky_front.png";
    "resources/sky_back.png";
)
set(RESOURCES_TEXTURES)

foreach(TEXTURE ${TEXTURES})
    get_filename_component(FILE_NAME ${TEXTURE} NAME)
    set(TEXTURE_OUT "${PROJECT_BINARY_DIR}/resources/${FILE_NAME}")
    add_custom_command(
        OUTPUT ${TEXTURE_OUT}
        COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/${TEXTURE}" ${TEXTURE_OUT}
        MAIN_DEPENDENCY "${PROJECT_SOURCE_DIR}/${TEXTURE}"
    )
    list(APPEND RESOURCES_TEXTURES ${TEXTURE_OUT})
endforeach(TEXTURE)

add_custom_target(
    CopyTextures
    DEPENDS ${RESOURCES_TEXTURES}
)

add_dependencies("Game" CompileShaders)
add_dependencies("Game" CopyTextures)

set_property(TARGET "Game" PROPERTY CXX_STANDARD 17)