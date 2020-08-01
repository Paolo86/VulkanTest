#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "CommonStructs.h"

#include "GraphicsPipeline.h"
#include "PushConstant.h"
#include "TextureSampler.h"


class Material
{
public:
	Material(std::string shaderName);
	~Material();
	void Create();
	void Destroy();


	VkPipelineLayout m_pipelineLayout; //Used to pass data to shaders (like mat4)
	GraphicsPipeline m_graphicsPipeline;
	PushConstant m_pushConstant;

	std::vector<VkDescriptorSetLayout> m_orderedDescriptorLayouts;
	//For UBO
	VkDescriptorSetLayout m_descriptorLayout;
	std::vector<VkDescriptorSet> m_descriptorSets;

	VkDescriptorSetLayout m_samplerDescriptorLayout;
	std::vector<VkDescriptorSet> m_samplerDescriptorSets;
	std::vector<VkImage> m_textureImages;
	std::vector<VkDeviceMemory> m_textureImagesMemory;
	std::vector<VkImageView> m_textureImagesViews;
	VkPushConstantRange m_pushContantRange;


	void CreateGraphicsPipeline();
	void CreateDescriptorSetLayout();
	void CreateDescriptorSets();
	int CreateTexture(std::string fileName);
	int CreateTextureImage(std::string fileName);
	int CreateTextureDescriptor(VkImageView textureImage);

	std::string m_shaderName;

};