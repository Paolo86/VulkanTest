#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vk_mem_alloc.h>

#include "CommonStructs.h"
#include "Mesh.h"
#include "stb_image.h"
#include "UniformBuffer.h"
#include "VkDebugMessanger.h"
#include "TextureSampler.h"


class VkContext
{
public:
	void Init();
	void CreateInstance();
	void SetUpDebugMessenger(); // Needs vkInstance so call after creating it
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSwapChain();
	void CreateImageViews();
	void CreateCommandPool();
	void CreatecommandBuffers();
	void CreateSynch();
	void CreateDescriptorPool();

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availabeFormats);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	bool AreValidationLayersSupported();
	std::vector<const char*> GetRequiredExtensions();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	bool IsDeviceSuitable(VkPhysicalDevice device);

private:
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkInstance m_vkInstance;
	VkFormat m_swapChainImageFormat;
	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;
	VkSwapchainKHR m_swapchain;
	VkQueue m_presentationQ;

	VkExtent2D m_swapChainExtent;
	VkDevice m_device;
	VkRenderPass m_renderPass;
	VkQueue m_graphicsQ;
	VkCommandPool m_commandPool;
	VkDescriptorPool m_descriptorPool;
	VkDescriptorPool m_samplerDescriptorPool;
	VkPhysicalDevice m_physicalDevice; //Destroyed automatically when instance is gone
	VkSurfaceKHR m_surface;
	std::vector<VkCommandBuffer> m_commandBuffers;

	std::vector<VkSemaphore> imageAvailable;	// Image ready to be drawn to
	std::vector<VkSemaphore> renderFinished; // Image ready for screen presentation
	std::vector<VkFence> drawFences;

	VkDeviceSize m_minUniformBufferOffset;
	size_t m_modelUniformAlignment;
	std::vector<VkExtensionProperties> m_supportedInstanceExtensions;
	std::vector<const char*> m_validationLayers;


};