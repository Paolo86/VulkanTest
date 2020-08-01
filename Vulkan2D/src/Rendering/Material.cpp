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
	CreateDescriptorSetLayout();
	CreateTexture("wood.jpg");
	CreateGraphicsPipeline();
}

void Material::Destroy()
{
	for (size_t i = 0; i < m_textureImages.size(); i++)
	{
		vkDestroyImageView(Vk::Instance().m_device, m_textureImagesViews[i], nullptr);
		vkDestroyImage(Vk::Instance().m_device, m_textureImages[i], nullptr);
		vkFreeMemory(Vk::Instance().m_device, m_textureImagesMemory[i], nullptr);
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

	VkShaderModule vertShaderModule = VkUtils::CreateShadeModule(Vk::Instance().m_device, vertexShaderCode);
	VkShaderModule fragShaderModule = VkUtils::CreateShadeModule(Vk::Instance().m_device, fragmentShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = VkUtils::GetPipelineVertexShaderStage(vertShaderModule);
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = VkUtils::GetPipelineFragmentShaderStage(fragShaderModule);
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
	VkVertexInputBindingDescription bindigDescription = {};
	std::array<VkVertexInputAttributeDescription, 3> attributeDescription;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};

	Vertex::GetVertexAttributeDescription(&bindigDescription, &vertexInputInfo, attributeDescription);	
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = VkUtils::GetPipelineInputAssemblyState();
	VkViewport viewport = VkUtils::GetViewport(Vk::Instance().m_swapChainExtent.width, Vk::Instance().m_swapChainExtent.height);
	VkRect2D scissor = VkUtils::GetScissor(Vk::Instance().m_swapChainExtent.width, Vk::Instance().m_swapChainExtent.height);
	VkPipelineViewportStateCreateInfo viewportState = VkUtils::GetPipelineViewportState(&viewport, &scissor);
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = VkUtils::GetPipelineRasterizer();

	//For some AA, requires GPU feature to be enabled
	VkPipelineMultisampleStateCreateInfo multisampling = VkUtils::GetPipelineMultisampling();

	//Blending stuff, disabled for now
	VkPipelineColorBlendAttachmentState colorBlendAttachment = VkUtils::GetPipelineBlendAttachmentState();

	VkPipelineColorBlendStateCreateInfo colorBlending = VkUtils::GetPipelineColorBlendingState(&colorBlendAttachment);

	m_pushConstant.Create<UboModel>(VK_SHADER_STAGE_VERTEX_BIT);

	//m_orderedDescriptorLayouts.push_back(m_descriptorLayout);
	//m_orderedDescriptorLayouts.push_back(m_samplerDescriptorLayout);

	std::vector<VkPushConstantRange> ranges;
	ranges.push_back(m_pushConstant.m_vkPushConstant);
	//VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = VkUtils::GetPipelineLayout(m_orderedDescriptorLayouts, ranges);
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(m_orderedDescriptorLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = m_orderedDescriptorLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &m_pushContantRange;


	// Create Pipeline Layout
	VkResult result = vkCreatePipelineLayout(Vk::Instance().m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Pipeline Layout!");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = VkUtils::GetPipelineDepthStencilAttachmentState();

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




void Material::CreateDescriptorSetLayout()
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




int Material::CreateTexture(std::string fileName)
{
	int textureImgaLoc = CreateTextureImage(fileName);
	VkImageView imageView = Vk::Instance().CreateImageView(m_textureImages[textureImgaLoc], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	m_textureImagesViews.push_back(imageView);

	textureImgaLoc = CreateTextureImage("texture.jpg");
	imageView = Vk::Instance().CreateImageView(m_textureImages[textureImgaLoc], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	m_textureImagesViews.push_back(imageView);

	//Texture sampler descriptor set layout
	VkDescriptorSetLayoutBinding samplerLayoutBinding = VkUtils::GetDescriptorLayout(0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
	VkDescriptorSetLayoutBinding imagesLayoutBinding = VkUtils::GetDescriptorLayout(1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, &Vk::Instance().m_textureSampler.m_sampler);

	DescriptorSetLayout samplerLayout;
	samplerLayout.AddBinding({ samplerLayoutBinding,imagesLayoutBinding }).Create(Vk::Instance().m_device);
	m_orderedDescriptorLayouts.push_back(samplerLayout.m_descriptorLayout);

	m_samplerDescriptorSets.resize(1);
	m_samplerDescriptorSets[0].CreateDescriptorSet(Vk::Instance().m_device, { samplerLayout }, Vk::Instance().m_samplerDescriptorPool);

	m_samplerDescriptorSets[0].AssociateTextureSamplerCombo(Vk::Instance().m_device, m_textureImagesViews , 0, Vk::Instance().m_textureSampler.m_sampler);
	//int descriptorLoc = CreateTextureDescriptor(imageView);
	return 0;
}

int Material::CreateTextureImage(std::string fileName)
{
	int w, h;
	VkDeviceSize size;
	stbi_uc* imageData = Vk::Instance().LoadTexture(fileName, &w, &h, &size);
	VkBuffer staging;
	VkDeviceMemory stagingMemory;
	Vk::Instance().CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging, &stagingMemory);

	void* data;
	vkMapMemory(Vk::Instance().m_device, stagingMemory, 0, size, 0, &data);
	memcpy(data, imageData, static_cast<size_t>(size));
	vkUnmapMemory(Vk::Instance().m_device, stagingMemory);

	stbi_image_free(imageData);

	VkImage texImage;
	VkDeviceMemory texImageMemory;
	texImage = Vk::Instance().CreateImage(w, h, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texImageMemory);

	//Before copy, transition layout to optimal transfer
	Vk::Instance().TransitionImageLayout(Vk::Instance().m_graphicsQ, Vk::Instance().m_commandPool, texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	Vk::Instance().CopyImageBuffer(Vk::Instance().m_graphicsQ, Vk::Instance().m_commandPool, staging, texImage, w, h);

	//After copy, transfer layout to shader readable
	Vk::Instance().TransitionImageLayout(Vk::Instance().m_graphicsQ, Vk::Instance().m_commandPool, texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	m_textureImages.push_back(texImage);
	m_textureImagesMemory.push_back(texImageMemory);

	vkDestroyBuffer(Vk::Instance().m_device, staging, nullptr);
	vkFreeMemory(Vk::Instance().m_device, stagingMemory, nullptr);
	return m_textureImages.size() - 1;
}

int Material::CreateTextureDescriptor(VkImageView textureImage)
{
	/*VkDescriptorSet descriptorSet;

	VkDescriptorSetAllocateInfo setAllocateInfo = {};
	setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocateInfo.descriptorPool = Vk::Instance().m_samplerDescriptorPool;
	setAllocateInfo.descriptorSetCount = 1;
	setAllocateInfo.pSetLayouts = &m_orderedDescriptorLayouts[1];

	if (vkAllocateDescriptorSets(Vk::Instance().m_device, &setAllocateInfo, &descriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor sets");
	}

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageView = textureImage;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.sampler = Vk::Instance().m_textureSampler.m_sampler;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;


	//Image-sampler Descriptor
	std::vector< VkWriteDescriptorSet> setWrites = { descriptorWrite };
	vkUpdateDescriptorSets(Vk::Instance().m_device, setWrites.size(), setWrites.data(), 0, nullptr);

	m_samplerDescriptorSets.push_back(descriptorSet);
	return m_samplerDescriptorSets.size() - 1;*/
	return 0;
}