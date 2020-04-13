#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <optional>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;

	bool IsComplete()
	{
		return graphicsFamily.has_value(); //If queues have been assigned
	}
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
		std::vector<VkExtensionProperties> m_supportedExtensions;

		VkPhysicalDevice m_physicalDevice; //Destroyed automatically when instance is gone
		VkDevice m_device;
		VkQueue m_graphicsQ;
		

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

		void PickPhysicalDevice();
		void CreateLogicalDevice();


};