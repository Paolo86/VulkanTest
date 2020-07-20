#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <optional>
#include "Mesh.h"

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

		void Draw();


	private:

		Mesh firstMesh;

		static std::unique_ptr<Vk> m_instance;
		int currentFrame = 0;
		/*Vk specific*/
		std::vector<VkSemaphore> imageAvailable;	// Image ready to be drawn to
		std::vector<VkSemaphore> renderFinished; // Image ready for screen presentation
		std::vector<VkFence> drawFences;



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
		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews;
		std::vector<VkFramebuffer> m_swapChainFramebuffers;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;

		std::vector<VkCommandBuffer> m_commandBuffers;
		VkCommandPool m_commandPool;

		bool m_validationLayersEnabled;
		std::vector<const char*> m_validationLayers;

		VkRenderPass m_renderPass;
		VkPipelineLayout m_pipelineLayout; //Used to pass data to shaders (like mat4)
		VkPipeline m_graphicsPipeline;


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
		void CreateImageViews();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreatecommandBuffers();
		void RecordCommands();

		void CreateSynch();

		VkShaderModule CreateShadeModule(const std::vector<char>& code);

		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availabeFormat);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& ccapabilities);

};