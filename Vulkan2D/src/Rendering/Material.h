#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "CommonStructs.h"

#include "GraphicsPipeline.h"
#include "PushConstant.h"
#include "DescriptorSet.h"
#include "TextureSampler.h"
#include "Texture2D.h"


class Material
{
public:
	Material(std::string shaderName);
	~Material();
	void Create();
	void Destroy();
	void AddTextures(std::vector<std::string> fileNames);

	VkPipelineLayout m_pipelineLayout; //Used to pass data to shaders (like mat4)
	GraphicsPipeline m_graphicsPipeline;
	PushConstant m_pushConstant;

	std::vector<VkDescriptorSetLayout> m_orderedDescriptorLayouts;
	std::vector<DescriptorSet> m_UBOdescriptorSets;
	std::vector<DescriptorSet> m_samplerDescriptorSets;
	std::vector<Texture2D> m_textures;


	void CreateGraphicsPipeline();
	void CreateUBODescriptorSet();
	void CreateSamplerDescriptorSet();

	std::string m_shaderName;

};