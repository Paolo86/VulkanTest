#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Utils/Logger.h"
#include "Rendering/Window.h"
#include "Rendering/Vk.h"
#include "Core/Input.h"

float camSpeed = 0.1;
float mouseSensitivity = 0.3;
glm::vec3 camPos;
glm::vec3 camDir;
glm::vec3 camSide;
float yaw = -90;
float pitch = 0;

void UpdateCamera()
{


	camDir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	camDir.y = sin(glm::radians(pitch));
	camDir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	camSide = glm::cross(camDir, glm::vec3(0, 1, 0));

	if (Input::GetMouseDown(GLFW_MOUSE_BUTTON_2))
	{
		Input::SetCursorMode("hidden");
		yaw -= Input::GetDeltaMousePosX() * mouseSensitivity;
		pitch += Input::GetDeltaMousePosY() * mouseSensitivity;
	}
	else
		Input::SetCursorMode("normal");


	if (Input::GetKeyDown(GLFW_KEY_W))
	{
		camPos += camDir * camSpeed;
	}
	if (Input::GetKeyDown(GLFW_KEY_S))
	{
		camPos -= camDir * camSpeed;
	}
	if (Input::GetKeyDown(GLFW_KEY_A))
	{
		camPos -= camSide * camSpeed;
	}
	if (Input::GetKeyDown(GLFW_KEY_D))
	{
		camPos += camSide * camSpeed;
	}

	if (Input::GetKeyDown(GLFW_KEY_PAGE_UP))
		camSpeed += 0.0001;
	if (Input::GetKeyDown(GLFW_KEY_PAGE_DOWN))
	{
		camSpeed -= 0.0001;
		camSpeed = camSpeed <= 0 ? 0 : camSpeed;
	}


}


int main() {

	Window::Instance().Create();
	Input::Init();
	Input::SetCursorMode("normal");
	Vk::Instance().Init();

	float timer = 0;
	int fps = 0;

	while (!glfwWindowShouldClose(Window::Instance().GetWindow())) {
		Timer::Instance().StartTimer("FPS");
		glfwPollEvents();

		UpdateCamera();
		Vk::Instance().UpdateView(camPos, camPos + camDir);


		Vk::Instance().Draw();
		Input::Update();
		double t = Timer::Instance().StopTimer("FPS");
		timer += t;
		fps++;
		if (timer >= 1.0)
		{
			Logger::LogInfo("FPS: ", fps);
			timer = 0;
			fps = 0;
		}

	}
	
	//std::cin.get();
	Window::Instance().Destroy();
	Vk::Instance().Destroy();
	return 0;
}