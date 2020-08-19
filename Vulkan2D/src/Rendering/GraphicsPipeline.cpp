#include "GraphicsPipeline.h"
#include "../Utils/FileUtils.h"
#include "VkUtils.h"
#include "..\Rendering\VkContext.h"

void GraphicsPipeline::Create(
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
	uint32_t subpass)
{
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = vertexInputInfo;
	pipelineInfo.pInputAssemblyState = inputAssembly;
	pipelineInfo.pViewportState = viewportState;
	pipelineInfo.pRasterizationState = rasterizerCreateInfo;
	pipelineInfo.pMultisampleState = multisampling;
	pipelineInfo.pDepthStencilState = depthStencilCreateInfo; // Optional
	pipelineInfo.pColorBlendState = colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = *pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = subpass;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void GraphicsPipeline::CreateLayout(std::string name, std::vector< VkDescriptorSetLayoutBinding> bindings, uint32_t setN)
{
	DescriptorSetLayout layout;
	layout.setNumber = setN;
	layout.AddBinding(bindings).Create(VkContext::Instance().GetLogicalDevice());
	allLayouts[name] = layout;

	vkLayouts.push_back(layout.m_descriptorLayout);
}