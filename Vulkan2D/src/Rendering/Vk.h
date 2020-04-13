#pragma once
#include <vulkan/vulkan.hpp>

class Vk
{
	public:
		static Vk& Instance();
		~Vk();
		void Init();
		void Destroy();


	private:
		static std::unique_ptr<Vk> m_instance;

		/*Vk specific*/
		VkInstance m_vkInstance;
		std::vector<VkExtensionProperties> m_supportedExtensions;
};