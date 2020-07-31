#include "VkUtils.h"
#include "Vk.h"

VkPipelineShaderStageCreateInfo VkUtils::GetPipelineVertexShaderStage(VkShaderModule vertShaderModule)
{
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	return vertShaderStageInfo;

}

VkPipelineShaderStageCreateInfo VkUtils::GetPipelineFragmentShaderStage(VkShaderModule fragShaderModule)
{
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	return fragShaderStageInfo;
}

VkViewport VkUtils::GetViewport(uint32_t extentWidth, uint32_t extentHeight)
{
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extentWidth;
	viewport.height = (float)extentHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	return viewport;
}

VkRect2D VkUtils::GetScissor(uint32_t extentWidth, uint32_t extentHeight)
{
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent.width = extentWidth;
	scissor.extent.height = extentHeight;
	return scissor;
}

VkPipelineViewportStateCreateInfo VkUtils::GetPipelineViewportState(VkViewport* viewport, VkRect2D* scissor)
{
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = scissor;
	return viewportState;
}

VkPipelineRasterizationStateCreateInfo VkUtils::GetPipelineRasterizer(float lineWidth,VkPolygonMode polygonMode,VkCullModeFlags cullMode,
	VkFrontFace frontFace)
{
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;					// Change if fragments beyond near/far planes are clipped (default) or clamped to plane
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;			// Whether to discard data and skip rasterizer. Never creates fragments, only suitable for pipeline without framebuffer output
	rasterizerCreateInfo.polygonMode = polygonMode;			// How to handle filling points between vertices
	rasterizerCreateInfo.lineWidth = lineWidth;								// How thick lines should be when drawn
	rasterizerCreateInfo.cullMode = cullMode;				// Which face of a tri to cull
	rasterizerCreateInfo.frontFace = frontFace;	// Winding to determine which side is front
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
	return rasterizerCreateInfo;
}

VkPipelineMultisampleStateCreateInfo VkUtils::GetPipelineMultisampling()
{
	//For some AA, requires GPU feature to be enabled
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional
	return multisampling;
}

VkPipelineColorBlendAttachmentState VkUtils::GetPipelineBlendAttachmentState(
	bool blendEnable,
	VkBlendFactor srcColorBlendFactor,
	VkBlendFactor dstColorBlendFactor,
	VkBlendFactor srcAlphaBlendFactor,
	VkBlendFactor dstAlphaBlendFactor,
	VkBlendOp colorBlendOperation,
	VkBlendOp alphaBlendOperation)
{
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = blendEnable;
	colorBlendAttachment.srcColorBlendFactor = srcColorBlendFactor; // Optional
	colorBlendAttachment.dstColorBlendFactor = dstColorBlendFactor; // Optional
	colorBlendAttachment.colorBlendOp = colorBlendOperation; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = srcAlphaBlendFactor; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = dstAlphaBlendFactor; // Optional
	colorBlendAttachment.alphaBlendOp = alphaBlendOperation; // Optional
	return colorBlendAttachment;
}

VkPipelineColorBlendStateCreateInfo VkUtils::GetPipelineColorBlendingState(VkPipelineColorBlendAttachmentState* attachmentBlending)
{
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = attachmentBlending;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional
	return colorBlending;
}

VkPipelineDepthStencilStateCreateInfo VkUtils::GetPipelineDepthStencilAttachmentState(
	bool depthTestEnable,
	bool writeTestEnable,
	bool stencilTestEnable,
	VkCompareOp compareOperation)
{
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = depthTestEnable;
	depthStencilCreateInfo.depthWriteEnable = writeTestEnable;
	depthStencilCreateInfo.depthCompareOp = compareOperation;		//If less, overwrite
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;		//Does the depth value exists between 2 bounds, bounds can be defined
	depthStencilCreateInfo.stencilTestEnable = stencilTestEnable;
	return depthStencilCreateInfo;
}

VkPipelineInputAssemblyStateCreateInfo VkUtils::GetPipelineInputAssemblyState(VkPrimitiveTopology topology)
{
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	return inputAssembly;
}


VkShaderModule VkUtils::CreateShadeModule(VkDevice device, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); //The byte code pointer is of type uint32_t....weird

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

VkPipelineLayoutCreateInfo VkUtils::GetPipelineLayout(
	std::vector<VkDescriptorSetLayout>& descriptorLayouts,
	std::vector<VkPushConstantRange>& pushConstants
	)
{
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
	pipelineLayoutCreateInfo.pPushConstantRanges = pushConstants.data();
	return pipelineLayoutCreateInfo;
}



