#pragma once
#include <string>
#include <GLFW\glfw3.h>
#include "..\Utils\Logger.h"
#include "..\Rendering\Window.h"

/**
* @class Input
* @brief Allows for retreiving of GLFW keystates
*
*
*Contains functions to determine the current state of keys
*
* @author Dylan Green
* @version 01
* @date 15/03/2019
*
*
* @bug No known bugs.
*/
class Input
{
public:
	//---- Public Member Functions ----//
		//-- GLFW --//
		/**
		* @brief		Initialises the glfwSetKeyCallback and keys array, and optionally the glfwSetErrorCallback
		*
		* @pre			GLFW must be initialised
		* @post			glfwSetKeyCallback is intialised in reference to the window input
		*
		* @param		window			GLFWwindow pointer to reference for the KeyCallback
		* @param		disableCursor	Sets the cursor to GLFW_CURSOR_DISABLED
		* @param		logGlfwErrors		Enables glfwSetErrorCallback, which logs glfw errors to the Logger
		*/
	static void Init(bool disableCursor = false, bool logGlfwErrors = false);

	/**
	* @brief		Calls glfwPollEvents(), which updates the states of the keys, this should be called regularly
	*/
	static void Update();

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		//-- Setters --//
		/**
		* @brief		Sets the cursor mode to the specified value
		*
		* @pre			GLFW must be initialised
		* @post			glfwSetInputMode(window, GLFW_Cursor, mode) is called with 'mode' being the mode based on the string parameter
		*
		* @param		mode		Sets the cursor mode to "normal", "hidden", or "disabled" based on input, value must be lowercase
		*/
	static void SetCursorMode(std::string mode);

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		//-- Accessors --//
		/**
		* @brief		Returns whether the key specified equals GLFW_PRESS
		*
		* @pre			The GameObject must exist.
		* @post			The true or false, depending on the value stored in keys[key] will be retreived
		*
		* @return		true if keys[key] equals GLFW_PRESS, else false
		*/
	static bool GetKeyPressed(int key);

	/**
	* @brief		Returns whether the key specified was released within the last update
	*
	* @pre			The GameObject must exist.
	* @post			The true or false, depending on the value stored in prevKeys[keys] and keys[key] will be retreived
	*
	* @return		true if prevKeys[key] equals GLFW_PRESS or GLFW_REPEAT and keys[key] equals GLFW_REALEASE, else false
	*/
	static bool GetKeyReleased(int key);

	/**
	* @brief		Returns whether the key specified equals GLFW_REPEAT
	*
	* @pre			The GameObject must exist.
	* @post			The true or false, depending on the value stored in keys[key] will be retreived
	*
	* @return		true if keys[key] equals GLFW_REPEAT, else false
	*/
	static bool GetKeyDown(int key);

	/**
	* @brief		Returns whether the key specified equals GLFW_RELEASE
	*
	* @pre			The GameObject must exist.
	* @post			The true or false, depending on the value stored in keys[key] will be retreived
	*
	* @return		true if keys[key] equals GLFW_RELEASE, else false
	*/
	static bool GetKeyUp(int key);

	/**
	* @brief		Returns whether the mouse button specified equals GLFW_PRESS
	*
	* @pre			The GameObject must exist.
	* @post			The true or false, depending on the value stored in mouseButtons[button] will be retreived
	*
	* @return		true if mouseButtons[button] equals GLFW_PRESS, else false
	*/
	static bool GetMouseDown(int button);

	/**
	* @brief		Returns whether the mouse button specified equals GLFW_RELEASE
	*
	* @pre			The GameObject must exist.
	* @post			The true or false, depending on the value stored in mouseButtons[button] will be retreived
	*
	* @return		true if mouseButtons[button] equals GLFW_RELEASE, else false
	*/
	static bool GetMouseUp(int button);

	/**
	* @brief		Returns the location of the mouse along the x-axis
	*
	* @pre			The GameObject must exist.
	* @post			The position of the mouse along the x-axis will be retreived
	*
	* @return		a double representing the position of the mouse along the x-axis
	*/
	static double GetMousePosX();

	/**
	* @brief		Returns the location of the mouse along the y-axis
	*
	* @pre			The GameObject must exist.
	* @post			The position of the mouse along the y-axis will be retreived
	*
	* @return		a double representing the position of the mouse along the y-axis
	*/
	static double GetMousePosY();

	/**
	* @brief		Returns the diffence between the last polled position of the mouse along the x-axis and the current position
	*
	* @pre			The GameObject must exist.
	* @post			The difference between the current and last positions of the mouse along the x-axis will be retreived
	*
	* @return		a double representing the delta of the mouse along the x-axis (current - previous)
	*/
	static double GetDeltaMousePosX();

	/**
	* @brief		Returns the diffence between the last polled position of the mouse along the y-axis and the current position
	*
	* @pre			The GameObject must exist.
	* @post			The difference between the current and last positions of the mouse along the y-axis will be retreived
	*
	* @return		a double representing the delta of the mouse along the y-axis (current - previous)
	*/
	static double GetDeltaMousePosY();

	/**
	* @brief		Returns whether the cursor is currently in the window
	*
	* @pre			The GameObject must exist.
	* @post			A boolean representing whether the cursor is in the window currently will be returned
	*
	* @return		true if the cursor is in the window currently, otherwise false
	*/
	static bool GetCursorInWindow();


	static void SetIsEnabled(bool s) { isEnabled = s; }

	static bool GetIsEnabled() { return isEnabled; }


private:
	static GLFWwindow* window;
	static int prevKeys[400]; //Stores the previous value of the keys
	static int keys[400]; //Stores the last updated value of the keys
	static int mouseButtons[8]; //Stores the last update value of the mouse buttons

	static double mouseX; //Position of the mouse along the X-axis
	static double mouseY; //Position of the mouse along the Y-axis
	static double deltaMouseX; //Delta between this positon and the last position of the mouse along the X-axis
	static double deltaMouseY; //Delta between this positon and the last position of the mouse along the Y-axis

	static bool cursorInWindow; //Whether the cursor is currently in the window

	static bool isEnabled;
	//---- Private Member Functions ----//
		//-- GLFW Callbacks --//
		/**
		* @brief		Callback for glfwSetKeyCallback
		*/
	static void Key_Callback(GLFWwindow* window, int, int, int, int);

	/**
	* @brief		Callback for glfwSetCursorPosCallback
	*/
	static void Cursor_Pos_Callback(GLFWwindow* window, double xpos, double ypos);

	/**
	* @brief		Callback for glfwSetMouseButtonCallback
	*/
	static void Mouse_Button_Callback(GLFWwindow* window, int button, int action, int mods);

	/**
	* @brief		Callback for glfwSetCursorEnterCallback
	*/
	static void Cursor_Enter_Callback(GLFWwindow* window, int entered);

	/**
	* @brief		Callback for glfwSetErrorCallback
	*/
	static void Error_Callback(int error, const char* description);
};

