#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


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