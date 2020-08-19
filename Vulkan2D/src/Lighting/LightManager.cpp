#include "LightManager.h"
#include "..\Rendering\VkContext.h"
#include "..\Rendering\Vk.h"

std::unique_ptr<LightManager> LightManager::m_instance;

LightManager& LightManager::Instance()
{
	if (!m_instance)
		m_instance = std::unique_ptr<LightManager>(new LightManager());

	return *m_instance;
}

void LightManager::BindDescriptorSet(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout) {

	m_lightDescriptorSet.Bind(cmdBuffer, pipelineLayout);
}


void LightManager::Init()
{


	m_directionalLights.diffuse[0] = glm::vec4(1.0, 1.0, 1.0, 1.0);
	m_directionalLights.direction[0] = glm::vec4(1.0, 0.0, 0.0, 0.0);
	m_directionalLights.specular[0] = glm::vec4(1.0, 1.0, 1.0, 1.0);
	m_directionalLights.intensity[0] = glm::vec4(1,1,1,1);

	m_directionalLights.diffuse[1] = glm::vec4(1.0,1.0,1.0,1.0);
	m_directionalLights.specular[1] = glm::vec4(1.0, 1.0, 1.0, 1.0);
	m_directionalLights.direction[1] = glm::vec4(-1.0,0.0,0.0,0.0);
	m_directionalLights.intensity[1] = glm::vec4(0.1);

	//Misc: 0 - intensity (need to change per light), 1 - count of lights
	m_directionalLights.misc = glm::vec4(1.0,1,0.0,0.0);

	m_directionalLigthsUBO.Create(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, 1);
	m_directionalLigthsUBO.Update(VkContext::Instance().GetLogicalDevice(), &m_directionalLights);

	//Light set binding
	VkDescriptorSetLayoutBinding lightBinding = VkUtils::PipelineUtils::GetDescriptorLayoutBinding(0,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT);

	DescriptorSetLayout layout;
	layout.setNumber = 0;
	layout.AddBinding({ lightBinding }).Create(VkContext::Instance().GetLogicalDevice());

	m_lightDescriptorSet.CreateDescriptorSet(VkContext::Instance().GetLogicalDevice(), layout, Vk::Instance().m_descriptorPool);
	m_lightDescriptorSet.AssociateUniformBuffers<DirectionalLightStruct>(VkContext::Instance().GetLogicalDevice(), { m_directionalLigthsUBO }, 0, 0);
	layout.Destroy(VkContext::Instance().GetLogicalDevice());
}

