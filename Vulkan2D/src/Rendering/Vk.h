#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>

class Vk
{
	public:
		static Vk& Instance();
		Vk();
		~Vk();
		void Init();
		void Destroy();

		void CreateInstance();
		void SetUpDebugMessenger();
		// Needed to apply debug messaging to create/destroy VkInstance
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);


	private:
		static std::unique_ptr<Vk> m_instance;

		/*Vk specific*/

		/*Params*/
		VkDebugUtilsMessengerEXT m_debugMessenger;

		VkInstance m_vkInstance;
		std::vector<VkExtensionProperties> m_supportedExtensions;
		

		bool m_validationLayersEnabled;
		std::vector<const char*> m_validationLayers;

		/*Methods*/
		bool AreValidationLayersSupported();
		std::vector<const char*> GetRequiredExtensions();

};