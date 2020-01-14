cmake_minimum_required (VERSION 3.8)

project ("Game")

add_executable("Game"
    main.cpp
)

target_include_directories("Game"
    PRIVATE "../Engine/include"
)

target_link_libraries("Game"
    "Engine"
)