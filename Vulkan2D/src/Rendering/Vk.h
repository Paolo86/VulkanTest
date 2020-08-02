#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "CommonStructs.h"
#include "Mesh.h"
#include "stb_image.h"
#include "UniformBuffer.h"
#include "TextureSampler.h"
#include "VkDebugMessanger.h"
#include <vk_mem_alloc.h>


class Vk
{
	public:
		static Vk& Instance();
		Vk();
		~Vk();
		void Init();
		void Destroy();

		void Draw();

		stbi_uc* LoadTexture(std::string fileName, int* width, int* height);

		//Change image layout

		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		VkDevice m_device;
		VkRenderPass m_renderPass;
		std::vector<VkImage> m_swapChainImages;
		std::vector<UniformBuffer<_ViewProjection>> m_VPUniformBuffers;
		VkQueue m_graphicsQ;
		VkCommandPool m_commandPool;
		TextureSampler m_textureSampler;
		VkDescriptorPool m_descriptorPool;
		VkDescriptorPool m_samplerDescriptorPool;
		VkPhysicalDevice m_physicalDevice; //Destroyed automatically when instance is gone
		VkDebugUtilsMessengerEXT m_debugMessenger;


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

		UboModel* m_modelTransferSpace;
		/*Params*/

		VkInstance m_vkInstance;
		std::vector<VkExtensionProperties> m_supportedInstanceExtensions;

		VkQueue m_presentationQ;
		VkSwapchainKHR m_swapchain;
		VkSurfaceKHR m_surface;
		std::vector<VkImageView> m_swapChainImageViews;
		std::vector<VkFramebuffer> m_swapChainFramebuffers;

		VkImage m_depthBufferImage;			//No need for multiple (like swap chain images). It's loked when is being used
		VkDeviceMemory m_depthBufferMemory;
		VkImageView m_depthBufferImageView;

;

		std::vector<VkCommandBuffer> m_commandBuffers;



		std::vector<VkImage> m_textureImages;
		std::vector<VkDeviceMemory> m_textureImagesMemory;
		std::vector<VkImageView> m_textureImagesViews;

		bool m_validationLayersEnabled;
		std::vector<const char*> m_validationLayers;
		_ViewProjection ViewProjection;


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
		void CreateDepthBufferImage();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreatecommandBuffers();
		void RenderCmds(uint32_t currentImage);
		void CreateDescriptorPool();
		void CreateUniformBuffers();
		void CreateSynch();
		void AllocateDynamicBufferTransferSpace();


		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availabeFormat);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& ccapabilities);
		VkFormat ChooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

		VkDeviceSize m_minUniformBufferOffset;
		size_t m_modelUniformAlignment;

};