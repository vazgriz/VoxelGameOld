#include <Engine/Engine.h>
#include <GLFW/glfw3.h>

Engine::Engine() {
    glfwInit();
}

Engine::~Engine() {
    glfwTerminate();
}

void Engine::run() {

}