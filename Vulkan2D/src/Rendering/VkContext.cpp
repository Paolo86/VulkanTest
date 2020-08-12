#include "VkContext.h"
#include "../Utils/Logger.h"
#include "Window.h"

#include <set>
#define NOMINMAX

#define VALIDATION_LAYERAS_ENABLED 1

namespace
{
	const std::vector<const char*> supportedDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const int MAX_FRAME_DRAWS = 2;
}

std::unique_ptr<VkContext> VkContext::m_instance;

VkContext& VkContext::Instance() {
	if (m_instance == nullptr)
		m_instance = std::make_unique<VkContext>();

	return *m_instance;
}

void VkContext::Init()
{
	CreateInstance();
	SetUpDebugMessenger(); // Needs vkInstance so call after creating it
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();
	CreateCommandPool();
	CreatecommandBuffers();
	CreateSynch();
}

void VkContext::Destroy()
{

	vkDeviceWaitIdle(m_device); //Wait for device to be idle before cleaning up (so won't clean commands currently on queue)

	vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
	vkDestroyDescriptorPool(m_device, m_samplerDescriptorPool, nullptr);

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		vkDestroySemaphore(m_device, renderFinished[i], nullptr);
		vkDestroySemaphore(m_device, imageAvailable[i], nullptr);
		vkDestroyFence(m_device, drawFences[i], nullptr);
	}
	//_aligned_free(m_modelTransferSpace);
	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	vkDestroyCommandPool(m_device, m_commandLargerPool, nullptr);


	for (auto imageView : m_swapChainImageViews) {
		vkDestroyImageView(m_device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	if (VALIDATION_LAYERAS_ENABLED)
	{
		DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
	}
	vkDestroyDevice(m_device, nullptr);
	vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);
	vkDestroyInstance(m_vkInstance, nullptr);
}

void VkContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr; // Optional
}


void VkContext::SetUpDebugMessenger()
{
	if (VALIDATION_LAYERAS_ENABLED)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		PopulateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
}

bool VkContext::AreValidationLayersSupported()
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

void VkContext::CreateInstance()
{
	if (VALIDATION_LAYERAS_ENABLED && !AreValidationLayersSupported())
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
	if (VALIDATION_LAYERAS_ENABLED)
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

std::vector<const char*> VkContext::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = Window::Instance().GetGLFWExtensions(glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (VALIDATION_LAYERAS_ENABLED) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void VkContext::PickPhysicalDevice()
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

void VkContext::CreateLogicalDevice()
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

	if (VALIDATION_LAYERAS_ENABLED) {
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
	vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQ);
	vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentationQ);
}

// Check if some extension are supported, in this case, swapchain extension
bool VkContext::CheckDeviceExtensionSupport(VkPhysicalDevice device)
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


QueueFamilyIndices VkContext::FindQueueFamilies(VkPhysicalDevice device)
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

bool VkContext::IsDeviceSuitable(VkPhysicalDevice device)
{
	bool result;
	VkPhysicalDeviceFeatures deviceFeatures;
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	bool basic = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
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

void VkContext::CreateSurface()
{
	if (Window::Instance().CreateVKSurface(m_vkInstance, m_surface) != VK_SUCCESS)
	{
		Logger::LogError("Failed to create surface (GLFW)");
		throw std::runtime_error("Failed to create surface (GLFW)");
	}
	else
		Logger::LogInfo("Successfully created surface");
}

SwapChainSupportDetails VkContext::QuerySwapChainSupport(VkPhysicalDevice device)
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


VkSurfaceFormatKHR VkContext::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availabeFormats)
{
	for (const auto& availableFormat : availabeFormats)
	{
		// Standard 8 bit unsigned integer per color (32 bit per pixel)
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}

	return availabeFormats[0]; // If we don't find the one we want, return the first available
}

VkPresentModeKHR VkContext::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { //That's triple buffering, if the swap chain queue is full the image
																	// coming in will replace the one in the q
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR; //This mode is guaranteed to be available
}

VkExtent2D VkContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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

void VkContext::CreateSwapChain()
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

void VkContext::CreateImageViews()
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

void VkContext::CreateCommandPool()
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

	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = 0; // Optiona

		if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandLargerPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}
}

void VkContext::CreatecommandBuffers()
{
	// One command for each frame buffer
	m_commandBuffers.resize(m_swapChainImages.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

	if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}



void VkContext::CreateSynch()
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

VkFormat VkContext::ChooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags)
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

void VkContext::WaitForFenceAndAcquireImage(uint32_t& imageIndex)
{
	vkWaitForFences(GetLogicalDevice(), 1, &drawFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(GetLogicalDevice(), 1, &drawFences[currentFrame]);


	vkAcquireNextImageKHR(GetLogicalDevice(), m_swapchain, UINT64_MAX, imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);
}

void VkContext::Present(uint32_t imageIndex)
{
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

