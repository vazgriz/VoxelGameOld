cmake_minimum_required (VERSION 3.8)

project ("Game")

add_executable("Game"
    main.cpp
    FrameRateCounter.h
    FrameRateCounter.cpp
    TriangleRenderer.h
    TriangleRenderer.cpp
)

target_include_directories("Game"
    PRIVATE "../Engine/include"
)

target_link_libraries("Game"
    "Engine"
)

set(SHADER_SOURCES
    "shaders/shader.vert" ;
    "shaders/shader.frag" ;
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

add_dependencies("Game" CompileShaders)