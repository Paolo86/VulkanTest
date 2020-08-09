#pragma once

#include <vulkan/vulkan.hpp>
#include <string>
#include "DescriptorSet.h"

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


	DescriptorSetLayout& GetSamplerLayout() { return allLayouts[1]; }

	VkPipeline m_graphicsPipeline;
	VkPipelineLayout m_pipelineLayout;
	std::vector<DescriptorSetLayout> allLayouts;



};