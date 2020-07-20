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

	while (!glfwWindowShouldClose(Window::Instance().GetWindow())) {
		glfwPollEvents();
		Vk::Instance().Draw();
	}
	
	//std::cin.get();
	Window::Instance().Destroy();
	Vk::Instance().Destroy();
	return 0;
}