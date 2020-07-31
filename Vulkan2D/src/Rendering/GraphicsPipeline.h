#pragma once

#include <vulkan/vulkan.hpp>
#include <string>

class GraphicsPipeline
{
public:
	GraphicsPipeline() {};
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

	void Destroy(VkDevice device);

	VkPipeline m_graphicsPipeline;

};