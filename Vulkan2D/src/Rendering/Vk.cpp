#include "Vk.h"
#include "Window.h"
#include "../Utils/Logger.h"
#include <set>

std::unique_ptr<Vk> Vk::m_instance;

// Callback for validation layer debugging messages
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	//std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	Logger::LogError("VK_VL:\n\t", pCallbackData->pMessage);

	return VK_FALSE;
}
// Used to create the debug messenger.
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

Vk& Vk::Instance()
{
	if (m_instance == nullptr)
		m_instance = std::make_unique<Vk>();

	return *m_instance;
}

Vk::Vk()
{
	//Initialize validation layers
	m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
	m_validationLayersEnabled = 1;
}


Vk::~Vk()
{

}


void Vk::Init()
{
	CreateInstance();
	SetUpDebugMessenger(); // Needs vkInstance so call after creating it
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
}

void Vk::Destroy()
{
	if (m_validationLayersEnabled)
	{
		DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
	}
	vkDestroyDevice(m_device, nullptr);
	vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);
	vkDestroyInstance(m_vkInstance, nullptr);
}


bool Vk::AreValidationLayersSupported()
{
	// Get instance layers
	uint32_t layersCount = 0;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
	std::vector<VkLayerProperties> instanceLayers(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, instanceLayers.data());

	//Check against the ones we defined
	for (auto layerName : m_validationLayers)
	{
		bool found = false;

		for (auto instanceLayer : instanceLayers)
		{
			if (strcmp(layerName, instanceLayer.layerName) == 0)
			{
				found = true;
				break;
			}

		}

		//If one layer was not found, return false
		if (!found)
			return false;
	}

	return true;

}

std::vector<const char*> Vk::GetRequiredExtensions() 
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = Window::Instance().GetGLFWExtensions(glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (m_validationLayersEnabled) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void Vk::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{	
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr; // Optional
}


void Vk::SetUpDebugMessenger()
{
	if (m_validationLayersEnabled)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		PopulateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
}


void Vk::CreateInstance()
{
	if (m_validationLayersEnabled && !AreValidationLayersSupported())
	{
		Logger::LogError("A validation layer was not found");
		throw std::runtime_error("Validation layer not found");
	}

	// Optional struct
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = Window::Instance().GetTitle().c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Vulkan 2D";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;


	//Not optional
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = GetRequiredExtensions();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	//Validation layer
	if (m_validationLayersEnabled)
	{
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
		createInfo.ppEnabledLayerNames = m_validationLayers.data();
		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;

	}

	if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS)
		Logger::LogError(__FUNCTION__, "Failed to create instance");

	// Populate supported extensions
	uint32_t extensionsCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
	m_supportedExtensions.resize(extensionsCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, m_supportedExtensions.data());

	//Uncomment to view the list of supported extensions
	//for (const auto& extension : m_supportedExtensions)
	//	Logger::Log(extension.extensionName);
}

bool Vk::IsDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	bool result =  deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		deviceFeatures.geometryShader;

	QueueFamilyIndices indices = FindQueueFamilies(device);

	result == result && indices.IsComplete();
	if (result)
		Logger::LogInfo("Selected GPU:", deviceProperties.deviceName);

	return result;
}


void Vk::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);

	std::vector<VkPhysicalDevice> availableDevices(deviceCount);
	vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, availableDevices.data());

	for (VkPhysicalDevice device : availableDevices)
	{
		if (IsDeviceSuitable(device))
		{
			m_physicalDevice = device;			
			break; //Select first one suitable
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE)
	{
		Logger::LogError("Failed to select GPU!");
		throw std::runtime_error("Failed to select GPU");
	}

	FindQueueFamilies(m_physicalDevice);
}

QueueFamilyIndices Vk::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i; //Grab the array index of that queue
		}

		//Presentation queue: find a queue (in any family) that can do presentation
		VkBool32 presentationSupported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentationSupported);

		if (presentationSupported)
			indices.presentFamily = i;

		if (indices.IsComplete())
			break;

		i++;
	}

	return indices;
}

void Vk::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}


	VkPhysicalDeviceFeatures deviceFeatures = {}; // Will complete

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;

	// This is unnecessary as they have been set globally in the instance
	// But for older Vulkan version, this was required as validation layers were separated between instance and device
	createInfo.enabledExtensionCount = 0;
	if (m_validationLayersEnabled) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
		createInfo.ppEnabledLayerNames = m_validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	// Get the queue handle out
	vkGetDeviceQueue(m_device, indices.graphicsFamily.value(),0,&m_graphicsQ);
	vkGetDeviceQueue(m_device, indices.presentFamily.value(),0,&m_presentationQ);
}

void Vk::CreateSurface()
{
	if (Window::Instance().CreateVKSurface(m_vkInstance, m_surface) != VK_SUCCESS)
	{
		Logger::LogError("Failed to create surface (GLFW)");
		throw std::runtime_error("Failed to create surface (GLFW)");
	}
	else
		Logger::LogInfo("Successfully created surface");
}



