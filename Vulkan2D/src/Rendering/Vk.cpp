#define NOMINMAX
#include "Vk.h"
#include "Window.h"
#include "../Utils/Logger.h"
#include <set>
#include <algorithm>
#include <cstdint>
#include "../Utils/FileUtils.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

std::unique_ptr<Vk> Vk::m_instance;

namespace 
{
	const std::vector<const char*> supportedDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	const int MAX_FRAME_DRAWS = 2;
	const int MAX_OBJECTS = 50;
}


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
	m_validationLayersEnabled = 0;
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

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = m_physicalDevice;
	allocatorInfo.device = m_device;
	allocatorInfo.instance = m_vkInstance;

	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateDepthBufferImage();
	CreateFramebuffers();
	CreateCommandPool();

	ViewProjection.projection = glm::perspective(glm::radians(60.0f), (float)m_swapChainExtent.width / m_swapChainExtent.height, 0.01f, 1000.0f);
	ViewProjection.view = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	ViewProjection.projection[1][1] *= -1;  //Vulkan inverts the Y axis...

	Vertex v1;
	v1.pos = glm::vec3(-0.5, -0.5, 0.0);
	v1.color = glm::vec3(1.0, 0.0, 0.0);

	Vertex v2;
	v2.pos = glm::vec3(0.5, -0.5, 0.0);
	v2.color = glm::vec3(0.0, 1.0, 0.0);

	Vertex v3;
	v3.pos = glm::vec3(0.5, 0.5, 0.0);
	v3.color = glm::vec3(0.0, 0.0, 1.0);

	Vertex v4;
	v4.pos = glm::vec3(-0.5, 0.5, 0.0);
	v4.color = glm::vec3(0.0, 0.0, 1.0);

	std::vector<Vertex> vertices = { v1,v2,v3, v4 };
	std::vector<uint32_t> indices = { 0,1,2,2,3,0 };

	firstMesh = Mesh(m_physicalDevice, m_device, m_graphicsQ, m_commandPool, vertices, indices);
	firstMesh.uboModel.model = glm::translate(firstMesh.uboModel.model, glm::vec3(0.2, 0, -0.1));

	secondMesh = Mesh(m_physicalDevice, m_device, m_graphicsQ, m_commandPool, vertices, indices);
	secondMesh.uboModel.model = glm::translate(secondMesh.uboModel.model, glm::vec3(1, 0, -5));
	secondMesh.uboModel.model = glm::rotate(secondMesh.uboModel.model, glm::radians(45.0f), glm::vec3(0, 0, 1));

	m_meshes.push_back(firstMesh);
	m_meshes.push_back(secondMesh);
	CreatecommandBuffers();
	//AllocateDynamicBufferTransferSpace(); //Not used, using push constant
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSetLayout();
	CreateDescriptorSets();

	CreateSynch();

}

void Vk::Destroy()
{
	firstMesh.DestroyVertexBuffer();
	secondMesh.DestroyVertexBuffer();
	vkDeviceWaitIdle(m_device); //Wait for device to be idle before cleaning up (so won't clean commands currently on queue)
	vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);

	vkDestroyImageView(m_device, m_depthBufferImageView, nullptr);
	vkDestroyImage(m_device, m_depthBufferImage, nullptr);
	vkFreeMemory(m_device, m_depthBufferMemory, nullptr);

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		vkDestroySemaphore(m_device, renderFinished[i], nullptr);
		vkDestroySemaphore(m_device, imageAvailable[i], nullptr);
		vkDestroyFence(m_device, drawFences[i], nullptr);
	}
	//_aligned_free(m_modelTransferSpace);
	vkDestroyCommandPool(m_device, m_commandPool, nullptr);

	for (auto framebuffer : m_swapChainFramebuffers) {
		vkDestroyFramebuffer(m_device, framebuffer, nullptr);
	}
	for (size_t i = 0; i < m_swapChainImages.size(); i++)
	{
		vkDestroyBuffer(m_device, m_VPuniformBuffer[i], nullptr);
		vkFreeMemory(m_device, m_VPuniformBufferMemory[i], nullptr);

		//vkDestroyBuffer(m_device, m_modelDynamicPuniformBuffer[i], nullptr);
		//vkFreeMemory(m_device, m_modelDynamicuniformBufferMemory[i], nullptr);
	}
	//vkDestroyDescriptorSetLayout(m_device, m_descriptorLayout, nullptr); //Destroy before pipeline

	vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	for (auto imageView : m_swapChainImageViews) {
		vkDestroyImageView(m_device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
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
	appInfo.apiVersion = VK_API_VERSION_1_1;


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
	m_supportedInstanceExtensions.resize(extensionsCount);
	m_supportedInstanceExtensions.resize(extensionsCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, m_supportedInstanceExtensions.data());

	//Uncomment to view the list of supported extensions
	//for (const auto& extension : m_supportedExtensions)
	//	Logger::Log(extension.extensionName);
}

// Check if some extension are supported, in this case, swapchain extension
bool Vk::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &count, availableExtensions.data());

	std::set<std::string> requiredExtensions(supportedDeviceExtensions.begin(), supportedDeviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();

}


bool Vk::IsDeviceSuitable(VkPhysicalDevice device)
{
	bool result;
	VkPhysicalDeviceFeatures deviceFeatures;
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	bool basic =  deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		deviceFeatures.geometryShader;

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	QueueFamilyIndices indices = FindQueueFamilies(device);

	//Swap chain
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}


	result = basic && indices.IsComplete() && extensionsSupported && swapChainAdequate;

	

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

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);

	m_minUniformBufferOffset = deviceProperties.limits.minUniformBufferOffsetAlignment;

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


	/*
		Interesting thing: if the graphics and presentation Q are the same index (likely), the index needs to be specified only once
		(Because we will create only one queue with that index).
		The index used in the struct below must be unique.
	*/
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
	createInfo.ppEnabledExtensionNames = supportedDeviceExtensions.data(); //Swapchain extension must be manually enabled
	createInfo.enabledExtensionCount = static_cast<uint32_t>(supportedDeviceExtensions.size());

	// This is unnecessary as they have been set globally in the instance
	// But for older Vulkan version, this was required as validation layers were separated between instance and device

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

SwapChainSupportDetails Vk::QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	//Capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

	// Format
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());

	}

	//Present mode
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}


VkSurfaceFormatKHR Vk::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availabeFormats)
{
	for (const auto& availableFormat : availabeFormats)
	{
		// Standard 8 bit unsigned integer per color (32 bit per pixel)
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}

	return availabeFormats[0]; // If we don't find the one we want, return the first available
}

VkPresentModeKHR Vk::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { //That's triple buffering, if the swap chain queue is full the image
																	// coming in will replace the one in the q
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR; //This mode is guaranteed to be available
}

VkExtent2D Vk::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { Window::Instance().GetWidth(), Window::Instance().GetHeight() };
		
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void Vk::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1; //Minimum number of images required, add 1 because why the fuck not

	//Make sure the number of images doesn't go over max allowed. That is, if max allowed is not 0, which means there is no max cap
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //Draw directly to image
																// Use VK_IMAGE_USAGE_TRANSFER_DST_BIT render images to a separate image first to perform operations

	//Cache swap chain info
	m_swapChainExtent = extent;
	m_swapChainImageFormat = surfaceFormat.format;

	QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);


	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	// Need to check if the images in the swap chain will be accessed by different queues
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;     //How many will be sharing
		createInfo.pQueueFamilyIndices = queueFamilyIndices;	//Who will be sharing
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //blending with other windows in the window system
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE; //Don't care about obscured pixel.
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
		Logger::LogError("Failed to create swap chain");
		throw std::runtime_error("failed to create swap chain!");
	}

	//Retrieve swap chain images
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapChainImages.data());
}

void Vk::CreateImageViews()
{
	m_swapChainImageViews.resize(m_swapChainImages.size());

	for (size_t i = 0; i < m_swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}

VkShaderModule Vk::CreateShadeModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); //The byte code pointer is of type uint32_t....weird

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		Logger::LogError("Failed to create shader");
		//throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void Vk::CreateDescriptorSets()
{
	m_descriptorSets.resize(m_swapChainImages.size());

	std::vector<VkDescriptorSetLayout> setLayouts(m_swapChainImages.size(), m_descriptorLayout);

	VkDescriptorSetAllocateInfo setAllocateInfo = {};
	setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocateInfo.descriptorPool = m_descriptorPool;
	setAllocateInfo.descriptorSetCount = static_cast<uint32_t>(m_swapChainImages.size());
	setAllocateInfo.pSetLayouts = setLayouts.data();

	//Allocate descriptor sets
	if (vkAllocateDescriptorSets(m_device, &setAllocateInfo, m_descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor sets");
	}

	for (size_t i = 0; i < m_swapChainImages.size(); i++)
	{
		VkDescriptorBufferInfo VPbufferInfo = {};
		VPbufferInfo.buffer = m_VPuniformBuffer[i];
		VPbufferInfo.offset = 0;
		VPbufferInfo.range = sizeof(_ViewProjection);

		VkWriteDescriptorSet mvpSetWrite = {};
		mvpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		mvpSetWrite.dstSet = m_descriptorSets[i];
		mvpSetWrite.dstBinding = 0;
		mvpSetWrite.dstArrayElement = 0;
		mvpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		mvpSetWrite.descriptorCount = 1;
		mvpSetWrite.pBufferInfo = &VPbufferInfo;

		//Model Descriptor

		//binding info
		//Not used, changed to push constant
		/*VkDescriptorBufferInfo modelBufferInfo = {};
		modelBufferInfo.buffer = m_modelDynamicPuniformBuffer[i];
		modelBufferInfo.offset = 0;
		modelBufferInfo.range = m_modelUniformAlignment;

		VkWriteDescriptorSet modelSetWrite = {};
		modelSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		modelSetWrite.dstSet = m_descriptorSets[i];
		modelSetWrite.dstBinding = 1;
		modelSetWrite.dstArrayElement = 0;
		modelSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		modelSetWrite.descriptorCount = 1;
		modelSetWrite.pBufferInfo = &modelBufferInfo;*/

		std::vector< VkWriteDescriptorSet> setWrites = { mvpSetWrite };
		vkUpdateDescriptorSets(m_device, setWrites.size(), setWrites.data() , 0, nullptr);
	}
}

void Vk::CreatePushConstantRange()
{
	m_pushContantRange.offset = 0;
	m_pushContantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	m_pushContantRange.size = sizeof(UboModel);
}

void Vk::CreateDescriptorSetLayout()
{
	//Binding info
	VkDescriptorSetLayoutBinding vpLayoutBinding = {};
	vpLayoutBinding.binding = 0; //Binding number, check in vert shader
	vpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vpLayoutBinding.descriptorCount = 1;
	vpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	vpLayoutBinding.pImmutableSamplers = nullptr;	

	/*VkDescriptorSetLayoutBinding modelLayoutBinding = {};
	modelLayoutBinding.binding = 1; //Binding number, check in vert shader
	modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	modelLayoutBinding.descriptorCount = 1;
	modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	modelLayoutBinding.pImmutableSamplers = nullptr;*/

	std::vector< VkDescriptorSetLayoutBinding> bindings = { vpLayoutBinding }; //modelLayoutBinding no longer used, using push constant
	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutCreateInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(m_device, &layoutCreateInfo, nullptr, &m_descriptorLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout");
}

void Vk::CreateDescriptorPool()
{
	VkDescriptorPoolSize VPpoolSize = {};
	VPpoolSize.descriptorCount = static_cast<uint32_t>(m_VPuniformBuffer.size());
	VPpoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	/*VkDescriptorPoolSize modelPoolSize = {};
	modelPoolSize.descriptorCount = static_cast<uint32_t>(m_modelDynamicPuniformBuffer.size());
	modelPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;*/

	std::vector< VkDescriptorPoolSize> poolSizes = { VPpoolSize  }; //modelPoolSize no longer used, using push constant

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = static_cast<uint32_t>(m_swapChainImages.size());
	poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolCreateInfo.pPoolSizes = poolSizes.data();

	if (vkCreateDescriptorPool(m_device, &poolCreateInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool");
	}
}


void Vk::CreateUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(_ViewProjection);

	//VkDeviceSize modelBufferSize = m_modelUniformAlignment * MAX_OBJECTS;

	m_VPuniformBuffer.resize(m_swapChainImages.size());
	m_VPuniformBufferMemory.resize(m_swapChainImages.size());

	//m_modelDynamicPuniformBuffer.resize(m_swapChainImages.size());
	//m_modelDynamicuniformBufferMemory.resize(m_swapChainImages.size());

	for (size_t i = 0; i < m_swapChainImages.size(); i++)
	{
		Vk::Instance().CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &m_VPuniformBuffer[i], &m_VPuniformBufferMemory[i]);
		//Vk::Instance().CreateBuffer(modelBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &m_modelDynamicPuniformBuffer[i], &m_modelDynamicuniformBufferMemory[i]);

	}
}


void Vk::CreateGraphicsPipeline()
{
	auto vertexShaderCode = FileUtils::ReadFile("Shaders/vertexShader.spv");
	auto fragmentShaderCode = FileUtils::ReadFile("Shaders/fragmentShader.spv");

	VkShaderModule vertShaderModule = CreateShadeModule(vertexShaderCode);
	VkShaderModule fragShaderModule = CreateShadeModule(fragmentShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// Vertex input
	//Data for single vertex (including position, color, normals and so on)
	VkVertexInputBindingDescription bindigDescription = {};
	bindigDescription.binding = 0;
	bindigDescription.stride = sizeof(Vertex);
	bindigDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;		// Move onto next vertex
																	// VK_VERTEX_INPUT_RATE_INSTANCE: move to vertex of next instance
	// Data within the vertex
	std::array<VkVertexInputAttributeDescription, 2> attributeDescription;
	attributeDescription[0].binding = 0;
	attributeDescription[0].location = 0;
	attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescription[0].offset = offsetof(Vertex, pos);

	attributeDescription[1].binding = 0;
	attributeDescription[1].location = 1;
	attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescription[1].offset = offsetof(Vertex, color);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindigDescription; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data(); // Optional

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_swapChainExtent.width;
	viewport.height = (float)m_swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; //if true, Clamp far pixels instead of discarding
	rasterizer.rasterizerDiscardEnable = VK_FALSE; //If true, basically no rendering
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	//For some AA, requires GPU feature to be enabled
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	//Blending stuff, disabled for now
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	//Some stages of the pipeline can be modified without recreating the whole pipeline again
	VkDynamicState dynamicStates[] = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	CreatePushConstantRange();

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; // Optional
	pipelineLayoutInfo.pSetLayouts = &m_descriptorLayout; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
	pipelineLayoutInfo.pPushConstantRanges = &m_pushContantRange; // Optional

	if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilCreateInfo.depthCompareOp - VK_COMPARE_OP_LESS;		//If less, overwrite
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;		//Does the depth value exists between 2 bounds, bounds can be defined
	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
	depthStencilCreateInfo.minDepthBounds = 0.0f; // Optional
	depthStencilCreateInfo.maxDepthBounds = 1.0f; // Optional

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencilCreateInfo; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
	vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
}

void Vk::CreateRenderPass()
{
	//Attachments

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //Before rendering, clear
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //After rendering, store
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //We don't care how the layout was, we are going to clear it
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //Image ready for presentation

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = ChooseSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //Before rendering, clear
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; //After rendering, don't care
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //We don't care how the layout was, we are going to clear it
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; 

	//References
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	std::array<VkSubpassDependency, 2> subpassDependencies;

	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;

	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;

	std::array<VkAttachmentDescription, 2> renderPassAttachments = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachments.size());
	renderPassInfo.pAttachments = renderPassAttachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassInfo.pDependencies = subpassDependencies.data();

	if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

VkFormat Vk::ChooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags)
{
	for (VkFormat format : formats)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &properties);

		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
	}

	throw std::runtime_error("Failed to find a matching format for image");
}


void Vk::CreateDepthBufferImage()
{
	VkFormat depthFormat = ChooseSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	m_depthBufferImage = CreateImage(m_swapChainExtent.width, m_swapChainExtent.height, depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_depthBufferMemory);

	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = m_depthBufferImage;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = depthFormat;

	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(m_device, &createInfo, nullptr, &m_depthBufferImageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image views= for depth buffer!");
	}
	
}


void  Vk::CreateFramebuffers()
{
	m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

	for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {

		std::array<VkImageView, 2> attachments = {
			m_swapChainImageViews[i],
			m_depthBufferImageView
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_swapChainExtent.width;
		framebufferInfo.height = m_swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void Vk::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = 0; // Optiona

	if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void Vk::CreatecommandBuffers()
{
	// One command for each frame buffer
	m_commandBuffers.resize(m_swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

	if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void Vk::RecordCommands(uint32_t imageIndex)
{
	VkCommandBufferBeginInfo bufferBeginInfo = {};
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	//Info about render pass
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_renderPass;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = m_swapChainExtent;

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0.0,0.0,0.0, 1.0 };	//For attachment 0 (color)
	clearValues[1].depthStencil.depth = 1.0;			//For attachment 1 (depth)


	renderPassBeginInfo.pClearValues = clearValues.data();
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

	renderPassBeginInfo.framebuffer = m_swapChainFramebuffers[imageIndex];

	//Start recording
	VkResult result = vkBeginCommandBuffer(m_commandBuffers[imageIndex], &bufferBeginInfo);
	if (result)
	{
		throw std::runtime_error("Failed to start recording command buffer");
	}

	vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); //All render commands are primary

	//Bind pipeline to be used
	vkCmdBindPipeline(m_commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

	for (size_t j = 0; j < m_meshes.size(); j++)
	{
		VkBuffer vertexBuffer[] = { m_meshes[j].GetVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(m_commandBuffers[imageIndex], 0, 1, vertexBuffer, offsets);
		vkCmdBindIndexBuffer(m_commandBuffers[imageIndex], m_meshes[j].GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

		//uint32_t dynamicOffset = static_cast<uint32_t>(m_modelUniformAlignment * j);
		vkCmdPushConstants(m_commandBuffers[imageIndex], m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UboModel), &m_meshes[j].uboModel.model);
		vkCmdBindDescriptorSets(m_commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, 
			&m_descriptorSets[imageIndex], 0, nullptr);


		vkCmdDrawIndexed(m_commandBuffers[imageIndex], m_meshes[j].GetIndexCount(), 1, 0, 0, 0);
	}


	vkCmdEndRenderPass(m_commandBuffers[imageIndex]);


	//End recording
	result = vkEndCommandBuffer(m_commandBuffers[imageIndex]);
	if (result)
	{
		throw std::runtime_error("Failed to end recording command buffer");
	}
}

void Vk::CreateSynch()
{
	imageAvailable.resize(MAX_FRAME_DRAWS);
	renderFinished.resize(MAX_FRAME_DRAWS);
	drawFences.resize(MAX_FRAME_DRAWS);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; //Start signaled, otherwise the first draw call wil wait forever

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{

		if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &imageAvailable[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &renderFinished[i]) != VK_SUCCESS ||
			vkCreateFence(m_device, &fenceInfo, nullptr, &drawFences[i]) != VK_SUCCESS)
		{

			throw std::runtime_error("failed to create semaphores!");
		}
	}
}

void Vk::UpdateUBO(uint32_t imageIndex)
{

	void* data;
	vkMapMemory(m_device, m_VPuniformBufferMemory[imageIndex], 0, sizeof(_ViewProjection), 0, &data);
	memcpy(data, &ViewProjection, sizeof(_ViewProjection));
	vkUnmapMemory(m_device, m_VPuniformBufferMemory[imageIndex]);


	//Not used, using push constant instead
	/*for (size_t i = 0; i < m_meshes.size(); i++)
	{
		UboModel* model = (UboModel*)((uint64_t)m_modelTransferSpace + (i * m_modelUniformAlignment));
		*model = m_meshes[i].uboModel;
	}

	vkMapMemory(m_device, m_modelDynamicuniformBufferMemory[imageIndex], 0, m_modelUniformAlignment * m_meshes.size(), 0, &data);
	memcpy(data, m_modelTransferSpace, m_modelUniformAlignment * m_meshes.size());
	vkUnmapMemory(m_device, m_modelDynamicuniformBufferMemory[imageIndex]);*/
}

void Vk::Draw()
{
	vkWaitForFences(m_device, 1, &drawFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(m_device, 1, &drawFences[currentFrame]);


	// 1. Get next available image to draw to and set a signal when drawing is done (semaphore)
	// Get index of next image to be drawn to
	uint32_t imageIndex;

	vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

	RecordCommands(imageIndex);
	UpdateUBO(imageIndex);


	// 2. Submit command buffer to ququ for execution, make sure to wait for image to be signalled as available before drawing and signal back
	//		when it's done rendering


	VkSemaphore waitSemaphores[] = { imageAvailable[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitSemaphores = waitSemaphores; // One semaphore for each stage described in line above
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinished[currentFrame];		//When it's done drawing, signal the renderFinished sempahore

	if (vkQueueSubmit(m_graphicsQ, 1, &submitInfo, drawFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}
	// 3. Present to screen when the signal of end rendering comes

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	VkSwapchainKHR swapChains[] = { m_swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinished[currentFrame];
	presentInfo.pResults = nullptr; // Optional
	vkQueuePresentKHR(m_presentationQ, &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
}

uint32_t Vk::FindMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags flags)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((allowedTypes & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & flags) == flags) //Check if it's allowed type and all flags are ok
		{
			return i; //We want the index of the memory block
		}
	}
}

void Vk::AllocateDynamicBufferTransferSpace()
{

	m_modelUniformAlignment = (sizeof(UboModel) + m_minUniformBufferOffset - 1) & ~(m_minUniformBufferOffset - 1);
	//Fixed space ot hold all model matrices of all objects
	m_modelTransferSpace = (UboModel*)_aligned_malloc(m_modelUniformAlignment* MAX_OBJECTS, m_modelUniformAlignment);

}


void Vk::CreateBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags bufferProperties, VkBuffer* buffer, VkDeviceMemory* bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = bufferSize;
	bufferInfo.usage = usage;


	VkResult res = vkCreateBuffer(m_device, &bufferInfo, nullptr, buffer);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vertex buffer");
	}

	VkMemoryRequirements memRequirement = {};
	vkGetBufferMemoryRequirements(m_device, *buffer, &memRequirement);

	//Allocate memory
	VkMemoryAllocateInfo memoryAllocInfo = {};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = memRequirement.size;
	memoryAllocInfo.memoryTypeIndex = FindMemoryTypeIndex(memRequirement.memoryTypeBits, bufferProperties);//VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT: CPU can interact with memory
																					//VK_MEMORY_PROPERTY_HOST_COHERENT_BIT: Place data straight into buffer after mapping


	//Allocate memory to vk device memory
	res = vkAllocateMemory(m_device, &memoryAllocInfo, nullptr, bufferMemory);
	vkBindBufferMemory(m_device, *buffer, *bufferMemory, 0);
}

void Vk::CopyBuffer(VkQueue transferQueue, VkCommandPool transferPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
{
	VkCommandBuffer cmdBuffer;

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = transferPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(m_device, &allocInfo, &cmdBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //Use cmd buffer once

	vkBeginCommandBuffer(cmdBuffer, &beginInfo);

	VkBufferCopy bufferCopyRegion = {};
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = bufferSize;

	vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);
	vkEndCommandBuffer(cmdBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(transferQueue); // Wait for queue to be done, this avoids queue being overloaded if many copies are made

	vkFreeCommandBuffers(m_device, transferPool, 1, &cmdBuffer);
}

VkImage Vk::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags,
	VkMemoryPropertyFlags propFlags, VkDeviceMemory* outImageMemory)
{
	//Create Image
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.usage = useFlags;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		//Layout undefined initially, will get changed by render pass later if needed
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkImage image;
	if (vkCreateImage(m_device, &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image");
	}
	   
	//Create memory
	VkMemoryRequirements memRequirement = {};
	vkGetImageMemoryRequirements(m_device, image, &memRequirement);

	VkMemoryAllocateInfo memAllocateInfo = {};
	memAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocateInfo.allocationSize = memRequirement.size;
	memAllocateInfo.memoryTypeIndex = FindMemoryTypeIndex(memRequirement.memoryTypeBits, propFlags);

	if (vkAllocateMemory(m_device, &memAllocateInfo, nullptr, outImageMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate memory for image");
	}

	// Connect mem to image
	vkBindImageMemory(m_device, image, *outImageMemory, 0);

	return image;
}






