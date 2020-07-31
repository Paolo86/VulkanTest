#pragma once
#include <vulkan/vulkan.hpp>

class PushConstant
{
public:
	PushConstant() {};

	template <class T>
	void Create(VkShaderStageFlags shaderStage, uint32_t offset = 0)
	{
		m_vkPushConstant.size = sizeof(T);
		m_vkPushConstant.offset = offset;
		m_vkPushConstant.stageFlags = shaderStage;
	}


	VkPushConstantRange m_vkPushConstant;
};
