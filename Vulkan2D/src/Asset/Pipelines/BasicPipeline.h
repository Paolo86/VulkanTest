#pragma once
#include "..\Rendering\GraphicsPipeline.h"
#include "..\Utils\FileUtils.h"
#include "..\Rendering\Vertex.h"
#include "..\Rendering\Vk.h"
#include "..\Rendering\VkContext.h"
#include "..\Rendering\CommonStructs.h"


/*

Basic pipeline

Set 0 - UboModel
Set 1 - TextureSamplers (count 5)
Push constant for _ViewProjection

*/
class BasicPipeline : public GraphicsPipeline
{
public:
	virtual ~BasicPipeline() {}
	BasicPipeline()
	{
		// Create basic pipeline
		std::string vertexShaderName = "basic_vertex.spv";
		std::string fragmentShaderName = "basic_fragment.spv";
		auto vertexShaderCode = FileUtils::ReadFile("Shaders/" + vertexShaderName);
		auto fragmentShaderCode = FileUtils::ReadFile("Shaders/" + fragmentShaderName);
		VkShaderModule vertShaderModule = VkUtils::PipelineUtils::CreateShadeModule(VkContext::Instance().GetLogicalDevice(), vertexShaderCode);
		VkShaderModule fragShaderModule = VkUtils::PipelineUtils::CreateShadeModule(VkContext::Instance().GetLogicalDevice(), fragmentShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = VkUtils::PipelineUtils::GetPipelineVertexShaderStage(vertShaderModule);
		VkPipelineShaderStageCreateInfo fragShaderStageInfo = VkUtils::PipelineUtils::GetPipelineFragmentShaderStage(fragShaderModule);
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
		VkVertexInputBindingDescription bindigDescription = {};
		std::array<VkVertexInputAttributeDescription, 3> attributeDescription;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};

		Vertex::GetVertexAttributeDescription(&bindigDescription, &vertexInputInfo, attributeDescription);
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = VkUtils::PipelineUtils::GetPipelineInputAssemblyState();
		VkViewport viewport = VkUtils::PipelineUtils::GetViewport(VkContext::Instance().GetSwapChainExtent().width, VkContext::Instance().GetSwapChainExtent().height);
		VkRect2D scissor = VkUtils::PipelineUtils::GetScissor(VkContext::Instance().GetSwapChainExtent().width, VkContext::Instance().GetSwapChainExtent().height);
		VkPipelineViewportStateCreateInfo viewportState = VkUtils::PipelineUtils::GetPipelineViewportState(&viewport, &scissor);
		VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = VkUtils::PipelineUtils::GetPipelineRasterizer(
			1.0f,
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE
		);

		//For some AA, requires GPU feature to be enabled
		VkPipelineMultisampleStateCreateInfo multisampling = VkUtils::PipelineUtils::GetPipelineMultisampling();

		//Blending stuff, disabled for now
		VkPipelineColorBlendAttachmentState colorBlendAttachment = VkUtils::PipelineUtils::GetPipelineBlendAttachmentState();

		VkPipelineColorBlendStateCreateInfo colorBlending = VkUtils::PipelineUtils::GetPipelineColorBlendingState(&colorBlendAttachment);

		PushConstant pc;
		pc.Create<UboModel>(VK_SHADER_STAGE_VERTEX_BIT);

		//Layout
		VkDescriptorSetLayoutBinding vpLayoutBinding = VkUtils::PipelineUtils::GetDescriptorLayoutBinding(0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_VERTEX_BIT);

		DescriptorSetLayout uboLayout;
		uboLayout.AddBinding({ vpLayoutBinding }).Create(VkContext::Instance().GetLogicalDevice());

		// Texture set
		VkDescriptorSetLayoutBinding imagesLayoutBinding = VkUtils::PipelineUtils::GetDescriptorLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
			, 5, VK_SHADER_STAGE_FRAGMENT_BIT);

		VkDescriptorSetLayoutBinding materialPropertiesBinding = VkUtils::PipelineUtils::GetDescriptorLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
			, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

		DescriptorSetLayout samplerLayout;
		samplerLayout.AddBinding({ imagesLayoutBinding, materialPropertiesBinding }).Create(VkContext::Instance().GetLogicalDevice());


		allLayouts.push_back(uboLayout);
		allLayouts.push_back(samplerLayout);

		std::vector<VkDescriptorSetLayout> vkLayouts = { uboLayout.m_descriptorLayout, samplerLayout.m_descriptorLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = VkUtils::PipelineUtils::GetPipelineLayoutInfo(vkLayouts, &pc.m_vkPushConstant);

		// Create Pipeline Layout
		VkResult result = vkCreatePipelineLayout(VkContext::Instance().GetLogicalDevice(), &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Pipeline Layout!");
		}

		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = VkUtils::PipelineUtils::GetPipelineDepthStencilAttachmentState();

		Create(
			VkContext::Instance().GetLogicalDevice(),
			shaderStages,
			&vertexInputInfo,
			&inputAssembly,
			&viewportState,
			&rasterizerCreateInfo,
			&multisampling,
			&depthStencilCreateInfo,
			&colorBlending,
			&m_pipelineLayout,
			Vk::Instance().m_renderPass,
			0);


		vkDestroyShaderModule(VkContext::Instance().GetLogicalDevice(), fragShaderModule, nullptr);
		vkDestroyShaderModule(VkContext::Instance().GetLogicalDevice(), vertShaderModule, nullptr);

		m_pipelineDescriptorSet.resize(VkContext::Instance().GetSwapChainImagesCount());
		//Create 3 descriptor sets for ubo
		for (size_t i = 0; i < VkContext::Instance().GetSwapChainImagesCount(); i++)
		{
			m_pipelineDescriptorSet[i].CreateDescriptorSet(VkContext::Instance().GetLogicalDevice(), { uboLayout }, Vk::Instance().m_descriptorPool);
			std::vector<UniformBuffer<_ViewProjection>> bufs = { Vk::Instance().m_VPUniformBuffers[i] };
			m_pipelineDescriptorSet[i].AssociateUniformBuffers<_ViewProjection>(VkContext::Instance().GetLogicalDevice(), bufs, 0, 0);


		}
	}

	virtual void Bind(VkCommandBuffer cmdBuffer, int imageIndex) override
	{
		Logger::LogInfo("\tBinding pipeline BASIC");
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

		uint32_t offset = 0;
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0,
			1,
			&m_pipelineDescriptorSet[imageIndex].m_descriptorSet, 0, nullptr);
	}

	virtual void Destroy(VkDevice device) override
	{
		vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
		for (DescriptorSetLayout layout : allLayouts)
		{
			vkDestroyDescriptorSetLayout(VkContext::Instance().GetLogicalDevice(), layout.m_descriptorLayout, nullptr); //Destroy before pipeline
		}

	}


	std::vector<DescriptorSet> m_pipelineDescriptorSet;

};