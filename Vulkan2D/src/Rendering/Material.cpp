#include "Material.h"
#include "..\Utils\FileUtils.h"
#include "VkUtils.h"
#include "Vk.h"

Material::Material(std::string shaderName)
{
	m_shaderName = shaderName;
}


void Material::Create()
{
	CreateUBODescriptorSet();
	AddTextures({ "wood.jpg","texture.jpg" });
	CreateGraphicsPipeline();
}

void Material::AddTextures(std::vector<std::string> fileNames)
{
	for (std::string name : fileNames)
	{
		int w, h;
		stbi_uc* imageData = Vk::Instance().LoadTexture(name, &w, &h);

		Texture2D t(w, h, 4, VK_FORMAT_R8G8B8A8_UNORM, imageData);
		stbi_image_free(imageData);
		m_textures.push_back(t);
	}

	CreateSamplerDescriptorSet();
}


void Material::Destroy()
{
	for (size_t i = 0; i < m_textures.size(); i++)
	{
		m_textures[i].Destroy();
	}

	for (VkDescriptorSetLayout layout : m_orderedDescriptorLayouts)
	{
		vkDestroyDescriptorSetLayout(Vk::Instance().m_device, layout, nullptr); //Destroy before pipeline
	}

	vkDestroyPipelineLayout(Vk::Instance().m_device, m_pipelineLayout, nullptr);
	m_graphicsPipeline.Destroy(Vk::Instance().m_device);
}


Material::~Material()
{

}

void Material::CreateGraphicsPipeline()
{
	std::string vertexShaderName = m_shaderName + "_vertex.spv";
	std::string fragmentShaderName = m_shaderName + "_fragment.spv";
	auto vertexShaderCode = FileUtils::ReadFile("Shaders/" + vertexShaderName);
	auto fragmentShaderCode = FileUtils::ReadFile("Shaders/" + fragmentShaderName);

	VkShaderModule vertShaderModule = VkUtils::PipelineUtils::CreateShadeModule(Vk::Instance().m_device, vertexShaderCode);
	VkShaderModule fragShaderModule = VkUtils::PipelineUtils::CreateShadeModule(Vk::Instance().m_device, fragmentShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = VkUtils::PipelineUtils::GetPipelineVertexShaderStage(vertShaderModule);
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = VkUtils::PipelineUtils::GetPipelineFragmentShaderStage(fragShaderModule);
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
	VkVertexInputBindingDescription bindigDescription = {};
	std::array<VkVertexInputAttributeDescription, 3> attributeDescription;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};

	Vertex::GetVertexAttributeDescription(&bindigDescription, &vertexInputInfo, attributeDescription);	
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = VkUtils::PipelineUtils::GetPipelineInputAssemblyState();
	VkViewport viewport = VkUtils::PipelineUtils::GetViewport(Vk::Instance().m_swapChainExtent.width, Vk::Instance().m_swapChainExtent.height);
	VkRect2D scissor = VkUtils::PipelineUtils::GetScissor(Vk::Instance().m_swapChainExtent.width, Vk::Instance().m_swapChainExtent.height);
	VkPipelineViewportStateCreateInfo viewportState = VkUtils::PipelineUtils::GetPipelineViewportState(&viewport, &scissor);
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = VkUtils::PipelineUtils::GetPipelineRasterizer();

	//For some AA, requires GPU feature to be enabled
	VkPipelineMultisampleStateCreateInfo multisampling = VkUtils::PipelineUtils::GetPipelineMultisampling();

	//Blending stuff, disabled for now
	VkPipelineColorBlendAttachmentState colorBlendAttachment = VkUtils::PipelineUtils::GetPipelineBlendAttachmentState();

	VkPipelineColorBlendStateCreateInfo colorBlending = VkUtils::PipelineUtils::GetPipelineColorBlendingState(&colorBlendAttachment);

	m_pushConstant.Create<UboModel>(VK_SHADER_STAGE_VERTEX_BIT);


	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = VkUtils::PipelineUtils::GetPipelineLayoutInfo(m_orderedDescriptorLayouts, &m_pushConstant.m_vkPushConstant);

	// Create Pipeline Layout
	VkResult result = vkCreatePipelineLayout(Vk::Instance().m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Pipeline Layout!");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = VkUtils::PipelineUtils::GetPipelineDepthStencilAttachmentState();

	m_graphicsPipeline.Create(
		Vk::Instance().m_device,
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


	vkDestroyShaderModule(Vk::Instance().m_device, fragShaderModule, nullptr);
	vkDestroyShaderModule(Vk::Instance().m_device, vertShaderModule, nullptr);
}


void Material::CreateUBODescriptorSet()
{
	//Uniform 
	//Binding info
	VkDescriptorSetLayoutBinding vpLayoutBinding = {};
	vpLayoutBinding.binding = 0; //Binding number, check in vert shader
	vpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vpLayoutBinding.descriptorCount = 1;
	vpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	vpLayoutBinding.pImmutableSamplers = nullptr;

	DescriptorSetLayout uboLayout;
	uboLayout.AddBinding({ vpLayoutBinding }).Create(Vk::Instance().m_device);

	m_orderedDescriptorLayouts.push_back(uboLayout.m_descriptorLayout);
	m_UBOdescriptorSets.resize(Vk::Instance().m_swapChainImages.size());
	//Create 3 descriptor sets for ubo
	for (size_t i = 0; i < Vk::Instance().m_swapChainImages.size(); i++)
	{
		m_UBOdescriptorSets[i].CreateDescriptorSet(Vk::Instance().m_device, { uboLayout }, Vk::Instance().m_descriptorPool);
		std::vector<UniformBuffer<_ViewProjection>> bufs = { Vk::Instance().m_VPUniformBuffers[i] };
		m_UBOdescriptorSets[i].AssociateUniformBuffers<_ViewProjection>(Vk::Instance().m_device, bufs, 0, 0);
	}

}


void Material::CreateSamplerDescriptorSet()
{

	//Texture sampler descriptor set layout
	VkDescriptorSetLayoutBinding imagesLayoutBinding = VkUtils::PipelineUtils::GetDescriptorLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		,m_textures.size(), VK_SHADER_STAGE_FRAGMENT_BIT);

	DescriptorSetLayout samplerLayout;
	samplerLayout.AddBinding({ imagesLayoutBinding }).Create(Vk::Instance().m_device);
	m_orderedDescriptorLayouts.push_back(samplerLayout.m_descriptorLayout);

	m_samplerDescriptorSets.resize(1);
	m_samplerDescriptorSets[0].CreateDescriptorSet(Vk::Instance().m_device, { samplerLayout }, Vk::Instance().m_samplerDescriptorPool);

	m_samplerDescriptorSets[0].AssociateTextureSamplerCombo(Vk::Instance().m_device, m_textures , 0, Vk::Instance().m_textureSampler.m_sampler);

}

