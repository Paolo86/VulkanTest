#pragma once

#include <vulkan/vulkan.hpp>
#include <string>
#include "DescriptorSet.h"
#include <unordered_map>

class GraphicsPipeline
{
public:
	GraphicsPipeline() {};
	virtual ~GraphicsPipeline() {};

	void Create(
		VkDevice& device,
		VkPipelineShaderStageCreateInfo* shaderStages,
		VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
		VkPipelineInputAssemblyStateCreateInfo* inputAssembly,
		VkPipelineViewportStateCreateInfo* viewportState,
		VkPipelineRasterizationStateCreateInfo* rasterizerCreateInfo,
		VkPipelineMultisampleStateCreateInfo* multisampling,
		VkPipelineDepthStencilStateCreateInfo* depthStencilCreateInfo,
		VkPipelineColorBlendStateCreateInfo* colorBlending,
		VkPipelineLayout* pipelineLayout,
		VkRenderPass &renderPass,
		uint32_t subpass);

	virtual void Destroy(VkDevice device) = 0;
	virtual void Bind(VkCommandBuffer cmdBuffer, int imageIndex) = 0;

	VkPipelineLayout& GetPipelineLayout() { return m_pipelineLayout; }
	DescriptorSetLayout& GetLayoutByName(std::string name) { return allLayouts[name]; }


protected:
	void CreateLayout(std::string name, std::vector< VkDescriptorSetLayoutBinding> bindings, uint32_t setN);
	VkPipeline m_graphicsPipeline;
	VkPipelineLayout m_pipelineLayout;
	std::unordered_map<std::string, DescriptorSetLayout> allLayouts;

	std::vector<VkDescriptorSetLayout> vkLayouts;	//Cache all layouts 



};