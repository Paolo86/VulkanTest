#pragma once
#include <vulkan/vulkan.hpp>
#include <string>

class VkUtils
{
	public:
		static VkPipelineShaderStageCreateInfo GetPipelineVertexShaderStage(VkShaderModule vertShaderModule);
		static VkPipelineShaderStageCreateInfo GetPipelineFragmentShaderStage(VkShaderModule fragShaderModule);
		static VkViewport GetViewport(uint32_t extentWidth, uint32_t extentHeight);
		static VkRect2D GetScissor(uint32_t extentWidth, uint32_t extentHeight);
		static VkPipelineViewportStateCreateInfo GetPipelineViewportState(VkViewport* viewport, VkRect2D* scissor);
		static VkPipelineRasterizationStateCreateInfo GetPipelineRasterizer(float lineWidth = 1.0f,
			VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL,
			VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT,
			VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE);
		static VkPipelineMultisampleStateCreateInfo GetPipelineMultisampling();

		static VkPipelineColorBlendAttachmentState GetPipelineBlendAttachmentState(
			bool blendEnable = true,
			VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			VkBlendOp colorBlendOperation = VK_BLEND_OP_ADD,
			VkBlendOp alphaBlendOperation = VK_BLEND_OP_ADD);

		static VkPipelineColorBlendStateCreateInfo GetPipelineColorBlendingState(VkPipelineColorBlendAttachmentState* attachmentBlending);

		static VkPipelineDepthStencilStateCreateInfo GetPipelineDepthStencilAttachmentState(
			bool depthTestEnable = true,
			bool writeTestEnable = true,
			bool stencilTestEnable = false,
			VkCompareOp compareOperation = VK_COMPARE_OP_LESS);

		static VkPipelineInputAssemblyStateCreateInfo GetPipelineInputAssemblyState(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		static VkShaderModule CreateShadeModule(VkDevice device, const std::vector<char>& code);
		static VkPipelineLayoutCreateInfo GetPipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorLayouts,
			std::vector<VkPushConstantRange>& pushConstants);
};