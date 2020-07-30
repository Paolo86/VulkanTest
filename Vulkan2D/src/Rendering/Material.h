#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

class Material
{
public:
	Material(std::string shaderName);
	~Material();
	void Create();
	void Destroy();


	VkPipelineLayout m_pipelineLayout; //Used to pass data to shaders (like mat4)
	VkSampler m_textureSampler;
	VkPipeline m_graphicsPipeline;

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