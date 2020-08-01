#include "VkUtils.h"
#include "Vk.h"

VkPipelineShaderStageCreateInfo VkUtils::PipelineUtils::PipelineUtils::GetPipelineVertexShaderStage(VkShaderModule vertShaderModule)
{
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	return vertShaderStageInfo;

}

VkPipelineShaderStageCreateInfo VkUtils::PipelineUtils::GetPipelineFragmentShaderStage(VkShaderModule fragShaderModule)
{
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	return fragShaderStageInfo;
}

VkViewport VkUtils::PipelineUtils::GetViewport(uint32_t extentWidth, uint32_t extentHeight)
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

VkRect2D VkUtils::PipelineUtils::GetScissor(uint32_t extentWidth, uint32_t extentHeight)
{
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent.width = extentWidth;
	scissor.extent.height = extentHeight;
	return scissor;
}

VkPipelineViewportStateCreateInfo VkUtils::PipelineUtils::GetPipelineViewportState(VkViewport* viewport, VkRect2D* scissor)
{
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = scissor;
	return viewportState;
}

VkPipelineRasterizationStateCreateInfo VkUtils::PipelineUtils::GetPipelineRasterizer(float lineWidth,VkPolygonMode polygonMode,VkCullModeFlags cullMode,
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

VkPipelineMultisampleStateCreateInfo VkUtils::PipelineUtils::GetPipelineMultisampling()
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

VkPipelineColorBlendAttachmentState VkUtils::PipelineUtils::GetPipelineBlendAttachmentState(
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

VkPipelineColorBlendStateCreateInfo VkUtils::PipelineUtils::GetPipelineColorBlendingState(VkPipelineColorBlendAttachmentState* attachmentBlending)
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

VkPipelineDepthStencilStateCreateInfo VkUtils::PipelineUtils::GetPipelineDepthStencilAttachmentState(
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

VkPipelineInputAssemblyStateCreateInfo VkUtils::PipelineUtils::GetPipelineInputAssemblyState(VkPrimitiveTopology topology)
{
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	return inputAssembly;
}


VkShaderModule VkUtils::PipelineUtils::CreateShadeModule(VkDevice device, const std::vector<char>& code)
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

VkPipelineLayoutCreateInfo VkUtils::PipelineUtils::GetPipelineLayout(
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


VkDescriptorSetLayoutBinding VkUtils::PipelineUtils::GetDescriptorLayout(uint32_t binding, 
	VkDescriptorType descriptorType, 
	uint32_t descriptorCount, 
	VkShaderStageFlags shaderStage, 
	const VkSampler* sampler)
{
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = binding; //Binding number, check in vert shader
	samplerLayoutBinding.descriptorType = descriptorType;
	samplerLayoutBinding.descriptorCount = descriptorCount;
	samplerLayoutBinding.stageFlags = shaderStage;
	samplerLayoutBinding.pImmutableSamplers = sampler;
	return samplerLayoutBinding;
}

uint32_t VkUtils::MemoryUtils::FindMemoryTypeIndex(VkPhysicalDevice m_physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties)
{
	// Get properties of physical device memory
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((allowedTypes & (1 << i))														// Index of memory type must match corresponding bit in allowedTypes
			&& (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)	// Desired property bit flags are part of memory type's property flags
		{
			// This memory type is valid, so return its index
			return i;
		}
	}
}


VkImage VkUtils::ImageUtils::CreateImage(VkPhysicalDevice m_physicalDevice, VkDevice m_device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags,
	VkMemoryPropertyFlags propFlags, VkDeviceMemory* outImageMemory)
{
	// CREATE IMAGE
	// Image Creation Info
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;						// Type of image (1D, 2D, or 3D)
	imageCreateInfo.extent.width = width;								// Width of image extent
	imageCreateInfo.extent.height = height;								// Height of image extent
	imageCreateInfo.extent.depth = 1;									// Depth of image (just 1, no 3D aspect)
	imageCreateInfo.mipLevels = 1;										// Number of mipmap levels
	imageCreateInfo.arrayLayers = 1;									// Number of levels in image array
	imageCreateInfo.format = format;									// Format type of image
	imageCreateInfo.tiling = tiling;									// How image data should be "tiled" (arranged for optimal reading)
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// Layout of image data on creation
	imageCreateInfo.usage = useFlags;									// Bit flags defining what image will be used for
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;					// Number of samples for multi-sampling
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;			// Whether image can be shared between queues

	// Create image
	VkImage image;
	VkResult result = vkCreateImage(m_device, &imageCreateInfo, nullptr, &image);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create an Image!");
	}

	// CREATE MEMORY FOR IMAGE
	// Get memory requirements for a type of image
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(m_device, image, &memoryRequirements);

	// Allocate memory using image requirements and user defined properties
	VkMemoryAllocateInfo memoryAllocInfo = {};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = memoryRequirements.size;
	memoryAllocInfo.memoryTypeIndex = VkUtils::MemoryUtils::FindMemoryTypeIndex(m_physicalDevice,memoryRequirements.memoryTypeBits, propFlags);

	result = vkAllocateMemory(m_device, &memoryAllocInfo, nullptr, outImageMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate memory for image!");
	}

	// Connect memory to image
	vkBindImageMemory(m_device, image, *outImageMemory, 0);

	return image;
}

VkImageView VkUtils::ImageUtils::CreateImageView(VkDevice m_device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image;											// Image to create view for
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;						// Type of image (1D, 2D, 3D, Cube, etc)
	viewCreateInfo.format = format;											// Format of image data
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;			// Allows remapping of rgba components to other rgba values
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	// Subresources allow the view to view only a part of an image
	viewCreateInfo.subresourceRange.aspectMask = aspectFlags;				// Which aspect of image to view (e.g. COLOR_BIT for viewing colour)
	viewCreateInfo.subresourceRange.baseMipLevel = 0;						// Start mipmap level to view from
	viewCreateInfo.subresourceRange.levelCount = 1;							// Number of mipmap levels to view
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;						// Start array level to view from
	viewCreateInfo.subresourceRange.layerCount = 1;							// Number of array levels to view

	// Create image view and return it
	VkImageView imageView;
	VkResult result = vkCreateImageView(m_device, &viewCreateInfo, nullptr, &imageView);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create an Image View!");
	}

	return imageView;
}