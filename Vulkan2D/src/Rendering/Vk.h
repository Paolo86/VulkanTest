#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <optional>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value(); //If queues have been assigned
	}
};

// Collect info about swapchain
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities; //Resolution of images in swapchain
	std::vector<VkSurfaceFormatKHR> formats; //Color depth
	std::vector<VkPresentModeKHR> presentModes; //Condition for swapping images
};

class Vk
{
	public:
		static Vk& Instance();
		Vk();
		~Vk();
		void Init();
		void Destroy();



	private:
		static std::unique_ptr<Vk> m_instance;

		/*Vk specific*/

		/*Params*/
		VkDebugUtilsMessengerEXT m_debugMessenger;

		VkInstance m_vkInstance;
		std::vector<VkExtensionProperties> m_supportedInstanceExtensions;


		VkPhysicalDevice m_physicalDevice; //Destroyed automatically when instance is gone
		VkDevice m_device;
		VkQueue m_graphicsQ;
		VkQueue m_presentationQ;
		VkSwapchainKHR m_swapchain;
		VkSurfaceKHR m_surface;
		

		bool m_validationLayersEnabled;
		std::vector<const char*> m_validationLayers;

		/*Methods*/
		bool AreValidationLayersSupported();
		std::vector<const char*> GetRequiredExtensions();

		bool IsDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

		void CreateInstance();
		void SetUpDebugMessenger();
		// Needed to apply debug messaging to create/destroy VkInstance
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		void CreateSurface();

		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();

		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availabeFormat);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& ccapabilities);

};