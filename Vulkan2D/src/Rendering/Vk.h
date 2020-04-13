#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>

class Vk
{
	public:
		static Vk& Instance();
		~Vk();
		void Init();
		void Destroy();


	private:
		static std::unique_ptr<Vk> m_instance;
		Vk();

		/*Vk specific*/

		/*Params*/
		VkInstance m_vkInstance;
		std::vector<VkExtensionProperties> m_supportedExtensions;

		bool m_validationLayersEnabled;
		std::vector<const char*> m_validationLayers;



		/*Methods*/
		bool AreValidationLayersSupported();

};