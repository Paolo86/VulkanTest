#include "Input.h"
#include "glm/glm.hpp"
//-----Public-----//
void Input::Init(bool disableCursor, bool logGlfwErrors)
{
	window = Window::Instance().GetWindow();

	if (logGlfwErrors == true)
	{
		Logger::LogInfo("Initialising GLFW Error Callback");
		glfwSetErrorCallback(Error_Callback);
	}

	Logger::LogInfo("Initialising GLFW Key Callback");
	glfwSetKeyCallback(window, Key_Callback);

	Logger::LogInfo("Initialising GLFW Cursor Pos Callback");
	glfwSetCursorPosCallback(window, Cursor_Pos_Callback);

	Logger::LogInfo("Initialising GLFW Mouse Button Callback");
	glfwSetMouseButtonCallback(window, Mouse_Button_Callback);

	Logger::LogInfo("Initialising GLFW Cursor Enter Callback");
	glfwSetCursorEnterCallback(window, Cursor_Enter_Callback);

	if (disableCursor == true)
	{
		SetCursorMode("disabled");
	}
	else
	{
		SetCursorMode("normal");
	}

	for (int i = 0; i < 400; i++)
	{
		prevKeys[i] = 0;
		keys[i] = 0;
	}

	for (int i = 0; i < 8; i++)
	{
		mouseButtons[i] = 0;
	}

	mouseX = 0;
	mouseY = 0;
	deltaMouseX = 0;
	deltaMouseY = 0;
	isEnabled = true;
	SetCursorMode("disabled");
	Logger::LogInfo("Input initialised");
}

void Input::Update()
{

	for (int i = 0; i < 400; i++)
	{
		prevKeys[i] = keys[i];
		if (keys[i] == GLFW_PRESS)
		{
			keys[i] = GLFW_REPEAT;
		}
	}

	deltaMouseX = 0;
	deltaMouseY = 0;


}

void Input::SetCursorMode(std::string mode)
{
	if (mode == "normal")
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	else if (mode == "hidden")
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}
	else if (mode == "disabled")
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else
	{
		Logger::LogWarning("Attempted to set invalid cursor mode!");
	}
}

bool Input::GetKeyPressed(int key)
{
	if (keys[key] == GLFW_PRESS)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Input::GetKeyReleased(int key)
{
	if ((prevKeys[key] == GLFW_PRESS || prevKeys[key] == GLFW_REPEAT) && keys[key] == GLFW_RELEASE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Input::GetKeyDown(int key)
{
	if (keys[key] == GLFW_PRESS || keys[key] == GLFW_REPEAT)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Input::GetKeyUp(int key)
{
	if (keys[key] == GLFW_RELEASE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Input::GetMouseDown(int button)
{
	if (mouseButtons[button] == GLFW_PRESS)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Input::GetMouseUp(int button)
{
	if (mouseButtons[button] == GLFW_RELEASE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

double Input::GetMousePosX()
{
	return mouseX;
}

double Input::GetMousePosY()
{
	return mouseY;
}

double Input::GetDeltaMousePosX()
{
	return deltaMouseX;
}

double Input::GetDeltaMousePosY()
{
	return deltaMouseY;
}

bool Input::GetCursorInWindow()
{
	return cursorInWindow;
}


//-----Private-----//
int Input::prevKeys[400];
int Input::keys[400];
int Input::mouseButtons[8];
GLFWwindow* Input::window;
double Input::mouseX;
double Input::mouseY;
double Input::deltaMouseX;
double Input::deltaMouseY;
bool Input::isEnabled;
bool Input::cursorInWindow;

void Input::Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (!isEnabled) return;

	if (key >= 0 && key < 400)
	{
		prevKeys[key] = keys[key];
		keys[key] = action;
	}
	else
	{
		Logger::LogWarning("Key_Callback went out of range with value ", key, "!");
	}
}

void Input::Cursor_Pos_Callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!isEnabled)
	{
		deltaMouseX = 0;
		deltaMouseY = 0;
		return;
	}

	deltaMouseX = mouseX - xpos;
	deltaMouseY = mouseY - ypos;

	deltaMouseX = abs(deltaMouseX) < 15 ? deltaMouseX : 15 * glm::sign(deltaMouseX);
	deltaMouseY = abs(deltaMouseY) < 15 ? deltaMouseY : 15 * glm::sign(deltaMouseY);


	mouseX = xpos;
	mouseY = ypos;
	if (!cursorInWindow)
	{
		Logger::LogWarning("Reset");

		//deltaMouseX = 0;
		//deltaMouseY = 0;
	}
}

void Input::Mouse_Button_Callback(GLFWwindow* window, int button, int action, int mods)
{
	if (!isEnabled) return;

	mouseButtons[button] = action;
}

void Input::Cursor_Enter_Callback(GLFWwindow* window, int entered)
{
	if (!isEnabled) return;

	if (entered) //Cursor has entered the window
	{
		cursorInWindow = true;
		//deltaMouseX = 0;
		//deltaMouseY = 0;
	}
	else //Cursor has left the window
	{
		cursorInWindow = false;
		//Logger::LogWarning("Reset in curesor enter");

		//deltaMouseX = 0;
		//deltaMouseY = 0;
	}
}

void Input::Error_Callback(int error, const char* description)
{
	Logger::LogError(description);
}