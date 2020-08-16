# VoxelGame

Source code for the game described in these blog posts: https://vazgriz.com/189/creating-minecraft-in-one-week-with-c-and-vulkan/

## Dependencies

Vulkan 1.2

[LunarG Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) for Vulkan 1.2

glslc.exe (part of LunarG Vulkan SDK)

[GLFW](https://github.com/glfw/glfw) 3.2 or higher, built for static linking

[VulkanWrapper](https://github.com/vazgriz/VulkanWrapper) (This is my own handwritten Vulkan wrapper)

[Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) 2.3.0

[OpenGL Mathematics \(GLM\)](https://github.com/g-truc/glm) 0.9.9.7

[EnTT](https://github.com/skypjack/entt) 3.2.2

[STB](https://github.com/nothings/stb) for stb_image.h

[FastNoise](https://github.com/Auburn/FastNoise) 0.4

## Build Instructions

This project uses CMake as it's build system.
There are multiple CMake variables that must be set for this project to build properly.

Variable | Description
------------ | -------------
GLFW_ROOT | Root folder of GLFW
GLFW_BUILD | Folder of GLFW binaries
VULKAN_ROOT | Root folder of Vulkan SDK (ie C:/VulkanSDK/1.2.135.0)
GLSL_COMPILER | Path to glslc.exe
VKW_ROOT | Root folder of VulkanWrapper
VKW_BUILD | Folder of VulkanWrapper binaries
VMA_INCLUDE | Source folder of Vulkan Memory Allocator (ie .../VulkanMemoryAllocator/src)
GLM_INCLUDE | Root folder of GLM
ENTT_INCLUDE | Source folder of EnTT (ie .../entt/src)
STB_INCLUDE | Root folder of STB
FASTNOISE_ROOT | Root folder of FastNoise
