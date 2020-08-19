#pragma once
#include "DirectionalLight.h"
#include "..\Rendering\UniformBuffer.h"
#include "..\Rendering\DescriptorSet.h"

#include <iostream>

struct DirectionalLightStruct
{
	glm::vec4 diffuse[4];
	glm::vec4 specular[4];
	glm::vec4 direction[4];
	glm::vec4 intensity[4];
	glm::vec4 misc;

};

class LightManager
{
public:
	static LightManager& Instance();

	void Init();
	void BindDescriptorSet(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout);

private:
	static std::unique_ptr<LightManager> m_instance;
	UniformBuffer<DirectionalLightStruct> m_directionalLigthsUBO;
	DirectionalLightStruct m_directionalLights;
	DescriptorSet m_lightDescriptorSet;
	
};