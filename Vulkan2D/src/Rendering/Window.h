#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vk_mem_alloc.h>
class Window
{
public:
	static Window& Instance();
	void Create(int width = 800, int height = 600, const char* title = "Vulkan 2D");
	void Destroy();
	void Update();
	int GetWidth();
	int GetHeight();
	std::string GetTitle();
	const char** GetGLFWExtensions(uint32_t& count);
	VkResult CreateVKSurface(VkInstance& vkInstance, VkSurfaceKHR& surface);
	GLFWwindow* GetWindow() { return m_glfwWindow; }
private:
	static std::unique_ptr<Window> m_instance;
	GLFWwindow* m_glfwWindow = nullptr;
	int m_width;
	int m_height;
	std::string m_title;
	bool initialized = false;
};