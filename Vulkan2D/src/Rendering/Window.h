#pragma once
#include <GLFW/glfw3.h>
#include <iostream>
class Window
{
public:
	static Window& Instance();
	void Create(int width = 800, int height = 600, const char* title = "Vulkan 2D");
	void Destroy();
	void Update();

private:
	static std::unique_ptr<Window> m_instance;
	GLFWwindow* m_glfwWindow = nullptr;
	int m_width;
	int m_height;
	bool initialized = false;
};