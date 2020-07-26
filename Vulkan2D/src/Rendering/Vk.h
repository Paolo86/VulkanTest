#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include "Mesh.h"

#include <vk_mem_alloc.h>


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
		uint32_t FindMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags flags);
		void CopyBuffer(VkQueue transferQueue, VkCommandPool transferPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize);

		void CreateBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags bufferProperties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
		VkImage CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags,
			VkMemoryPropertyFlags propFlags, VkDeviceMemory* outImageMemory);
private:

		Mesh firstMesh;
		Mesh secondMesh;
		std::vector<Mesh> m_meshes;
		static std::unique_ptr<Vk> m_instance;
		int currentFrame = 0;
		/*Vk specific*/
		std::vector<VkSemaphore> imageAvailable;	// Image ready to be drawn to
		std::vector<VkSemaphore> renderFinished; // Image ready for screen presentation
		std::vector<VkFence> drawFences;

		VmaAllocator allocator;

		struct _ViewProjection {

			glm::mat4 projection;
			glm::mat4 view;
			glm::mat4 model;

		} ViewProjection;

		UboModel* m_modelTransferSpace;
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

		VkImage m_depthBufferImage;			//No need for multiple (like swap chain images). It's loked when is being used
		VkDeviceMemory m_depthBufferMemory;
		VkImageView m_depthBufferImageView;

		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;

		std::vector<VkCommandBuffer> m_commandBuffers;
		VkCommandPool m_commandPool;


		//Descriptor set
		VkDescriptorSetLayout m_descriptorLayout;
		VkDescriptorPool m_descriptorPool;
		std::vector<VkDescriptorSet> m_descriptorSets;
		VkPushConstantRange m_pushContantRange;

		bool m_validationLayersEnabled;
		std::vector<const char*> m_validationLayers;

		VkRenderPass m_renderPass;
		VkPipelineLayout m_pipelineLayout; //Used to pass data to shaders (like mat4)
		VkPipeline m_graphicsPipeline;

		std::vector<VkBuffer> m_VPuniformBuffer;
		std::vector<VkDeviceMemory> m_VPuniformBufferMemory;

		std::vector<VkBuffer> m_modelDynamicPuniformBuffer;
		std::vector<VkDeviceMemory> m_modelDynamicuniformBufferMemory;


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
		void CreateDepthBufferImage();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreatecommandBuffers();
		void RecordCommands(uint32_t currentImage);
		void CreateDescriptorSetLayout();
		void CreatePushConstantRange();
		void CreateDescriptorPool();
		void CreateUniformBuffers();
		void CreateDescriptorSets();
		void CreateSynch();
		void AllocateDynamicBufferTransferSpace();
		void UpdateUBO(uint32_t imageIndex);

		VkShaderModule CreateShadeModule(const std::vector<char>& code);

		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availabeFormat);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& ccapabilities);
		VkFormat ChooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

		VkDeviceSize m_minUniformBufferOffset;
		size_t m_modelUniformAlignment;

};