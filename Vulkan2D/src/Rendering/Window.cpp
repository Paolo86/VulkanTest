#include "Window.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "..\Utils\Logger.h"

std::unique_ptr<Window> Window::m_instance;

Window& Window::Instance()
{
	if (m_instance == nullptr)
		m_instance = std::make_unique<Window>();

	return *m_instance;
}

void Window::Create(int width, int height, const char* title)
{
	if (!initialized)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

		m_width = width;
		m_height = height;
		m_title = title;
		initialized = true;
	}
	else
		Logger::LogWarning(__FUNCTION__,"Attempted to re initialize window");
}

void Window::Destroy()
{
	glfwDestroyWindow(m_glfwWindow);
	glfwTerminate();
}

void Window::Update()
{
	while (!glfwWindowShouldClose(m_glfwWindow)) {
		glfwPollEvents();
	}
}

int Window::GetWidth()
{
	return m_width;
}
int Window::GetHeight()
{
	return m_height;
}

std::string Window::GetTitle()
{
	return m_title;
}

const char** Window::GetGLFWExtensions(uint32_t& count)
{
	return glfwGetRequiredInstanceExtensions(&count);	
}

VkResult Window::CreateVKSurface(VkInstance& vkInstance, VkSurfaceKHR& surface)
{
	return glfwCreateWindowSurface(vkInstance, m_glfwWindow, nullptr, &surface);
}


