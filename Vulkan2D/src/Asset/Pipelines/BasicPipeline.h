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

		//Vertex Data
		std::vector<VkVertexInputBindingDescription> bindigDescriptions;

		std::vector<VkVertexInputAttributeDescription> attributeDescription;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};

		Vertex::GetVertexAttributeDescription(bindigDescriptions, &vertexInputInfo, attributeDescription,1);


		VkPipelineInputAssemblyStateCreateInfo inputAssembly = VkUtils::PipelineUtils::GetPipelineInputAssemblyState();
		VkViewport viewport = VkUtils::PipelineUtils::GetViewport(VkContext::Instance().GetSwapChainExtent().width, VkContext::Instance().GetSwapChainExtent().height);
		VkRect2D scissor = VkUtils::PipelineUtils::GetScissor(VkContext::Instance().GetSwapChainExtent().width, VkContext::Instance().GetSwapChainExtent().height);
		VkPipelineViewportStateCreateInfo viewportState = VkUtils::PipelineUtils::GetPipelineViewportState(&viewport, &scissor);
		VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = VkUtils::PipelineUtils::GetPipelineRasterizer();

		//For some AA, requires GPU feature to be enabled
		VkPipelineMultisampleStateCreateInfo multisampling = VkUtils::PipelineUtils::GetPipelineMultisampling();

		//Blending stuff, disabled for now
		VkPipelineColorBlendAttachmentState colorBlendAttachment = VkUtils::PipelineUtils::GetPipelineBlendAttachmentState();

		VkPipelineColorBlendStateCreateInfo colorBlending = VkUtils::PipelineUtils::GetPipelineColorBlendingState(&colorBlendAttachment);

		//This pipeline uses a push constant
		PushConstant pc;
		pc.Create<UboModel>(VK_SHADER_STAGE_VERTEX_BIT);

		//Light set binding
		VkDescriptorSetLayoutBinding lightBinding = VkUtils::PipelineUtils::GetDescriptorLayoutBinding(0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_VERTEX_BIT);

		//Layout binding viewprojection
		VkDescriptorSetLayoutBinding vpLayoutBinding = VkUtils::PipelineUtils::GetDescriptorLayoutBinding(0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_VERTEX_BIT);


		//Create layouts so they can be referenced (eg. the material needs the sampler layout to create the sampler descriptor set)
	   //Order is important for the layout, add them in the same order as in the shader

		// Texture set
		VkDescriptorSetLayoutBinding imagesLayoutBinding = VkUtils::PipelineUtils::GetDescriptorLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
			, 5, VK_SHADER_STAGE_FRAGMENT_BIT);

		VkDescriptorSetLayoutBinding materialPropertiesBinding = VkUtils::PipelineUtils::GetDescriptorLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
			, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

		//Order is important!
		//Each layout needs to match the set number in the shader
		CreateLayout("Lights", { lightBinding }, 0);							//Set 0
		CreateLayout("ViewProjection", { vpLayoutBinding },1);							//Set 1
		CreateLayout("Sampler", { imagesLayoutBinding, materialPropertiesBinding },2);	//Set 2

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
			m_pipelineDescriptorSet[i].CreateDescriptorSet(VkContext::Instance().GetLogicalDevice(), GetLayoutByName("ViewProjection"), Vk::Instance().m_descriptorPool);
			m_pipelineDescriptorSet[i].AssociateUniformBuffers<_ViewProjection>(VkContext::Instance().GetLogicalDevice(), { Vk::Instance().m_VPUniformBuffers[i] }, 0, 0);

		}
	}

	virtual void Bind(VkCommandBuffer cmdBuffer, int imageIndex) override
	{
		//Logger::LogInfo("\tBinding pipeline BASIC");
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
		m_pipelineDescriptorSet[imageIndex].Bind(cmdBuffer, m_pipelineLayout);

		
	}

	virtual void Destroy(VkDevice device) override
	{
		vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
		for (auto it = allLayouts.begin(); it != allLayouts.end(); it++)
		{
			vkDestroyDescriptorSetLayout(VkContext::Instance().GetLogicalDevice(), it->second.m_descriptorLayout, nullptr); //Destroy before pipeline
		}

	}


	std::vector<DescriptorSet> m_pipelineDescriptorSet; //Multiple, once for each command buffer (as many as swap chain images)

};