#pragma once
#include <vulkan/vulkan.hpp>
#include <string>
#include <vk_mem_alloc.h>
#include "UniformBuffer.h"

class VkUtils
{
	public:
		class PipelineUtils
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

			static VkPipelineLayoutCreateInfo GetPipelineLayoutInfo(std::vector<VkDescriptorSetLayout>& layouts, VkPushConstantRange* pushConstant);

			static VkPipelineDepthStencilStateCreateInfo GetPipelineDepthStencilAttachmentState(
				bool depthTestEnable = true,
				bool writeTestEnable = true,
				bool stencilTestEnable = false,
				VkCompareOp compareOperation = VK_COMPARE_OP_LESS);

			static VkPipelineInputAssemblyStateCreateInfo GetPipelineInputAssemblyState(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
			static VkShaderModule CreateShadeModule(VkDevice device, const std::vector<char>& code);

			static VkDescriptorSetLayoutBinding GetDescriptorLayoutBinding(uint32_t binding,
				VkDescriptorType descriptorType,
				uint32_t descriptorCount,
				VkShaderStageFlags shaderStage,
				const VkSampler* sampler = nullptr);
		};
		
		class ImageUtils
		{
		public:
			static VkImage CreateImage(VkPhysicalDevice m_physicalDevice, VkDevice m_device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags,
				VkMemoryPropertyFlags propFlags, VkDeviceMemory* outImageMemory);

			static VkImageView CreateImageView(VkDevice m_device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

			static void CopyImageBuffer(VkDevice device, VkQueue transferQueue,
				VkCommandPool transferPool, VkBuffer srcBuffer, VkImage image, uint32_t width, uint32_t height);

			static void TransitionImageLayout(VkDevice m_device, VkQueue queue, 
				VkCommandPool pool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
				
		};

		class MemoryUtils
		{
		public:
			static void InitVMA(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance);
			static uint32_t FindMemoryTypeIndex(VkPhysicalDevice m_physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties);
			static void CreateBuffer(VkDevice m_device, VkPhysicalDevice m_physicalDevice, VkDeviceSize bufferSize, VkBufferUsageFlags usage,
				VkMemoryPropertyFlags bufferProperties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);

			static void CreateBufferVMA(VkDeviceSize bufferSize, VkBufferUsageFlags usage,
				VmaMemoryUsage bufferProperties, VkBuffer* buffer, VmaAllocation* bufferMemory);

			static void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation) { vmaDestroyBuffer(allocator, buffer, allocation); }


			static void CopyBuffer(VkDevice device, VkQueue transferQueue, 
				VkCommandPool transferPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize);		

			static VmaAllocator allocator;
		private:
			static VkDevice device;
			static VkPhysicalDevice physicalDevice;
		};

		class CmdUtils
		{
		public:
			static VkCommandBuffer BeginCmdBufferSingleUsage(VkDevice m_device, VkCommandPool pool);
			static void EndCmdBuffer(VkDevice m_device, VkCommandPool pool, VkQueue sumitTo, VkCommandBuffer cmdBuffer);
		};
};


