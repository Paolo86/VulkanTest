#pragma once
#include <vulkan/vulkan.hpp>

class Vk
{
	public:
		static Vk& Instance();
		void Init();


	private:
		static std::unique_ptr<Vk> m_instance;

		/*Vk specific*/
		VkInstance m_vkInstance;
};