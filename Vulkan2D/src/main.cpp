#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include "Utils/Logger.h"
#include "Rendering/Window.h"
#include "Rendering/Vk.h"

int main() {

	Window::Instance().Create();

	Vk::Instance().Init();

	Window::Instance().Update();
	Window::Instance().Destroy();

	std::cin.get();
	return 0;
}