#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include "Utils/Logger.h"
#include "Rendering/Window.h"
#include "Rendering/Vk.h"

int main() {

	Window::Instance().Create();

	Vk::Instance().Init();
	glm::vec3 camPos;
	Vk::Instance().UpdateView(camPos, glm::vec3(0, 0, -1));
	float camSpeed = 0.1;
	while (!glfwWindowShouldClose(Window::Instance().GetWindow())) {
		glfwPollEvents();

		if (GetAsyncKeyState('W') & 0x8000)
		{
			camPos.z -= camSpeed;
			Vk::Instance().UpdateView(camPos, camPos + glm::vec3(0, 0, -1));
		}
		if (GetAsyncKeyState('S') & 0x8000)
		{
			camPos.z += camSpeed;
			Vk::Instance().UpdateView(camPos, camPos + glm::vec3(0, 0, -1));

		}
		if (GetAsyncKeyState('A') & 0x8000)
		{
			camPos.x -= camSpeed;
			Vk::Instance().UpdateView(camPos, camPos + glm::vec3(0, 0, -1));
		}
		if (GetAsyncKeyState('D') & 0x8000)
		{
			camPos.x += camSpeed;
			Vk::Instance().UpdateView(camPos, camPos + glm::vec3(0, 0, -1));
		}



		Vk::Instance().Draw();
	}
	
	//std::cin.get();
	Window::Instance().Destroy();
	Vk::Instance().Destroy();
	return 0;
}