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
	CreateDescriptorSets();
	CreateGraphicsPipeline();
	CreateTexture("wood.jpg");
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

	m_orderedDescriptorLayouts.push_back(m_descriptorLayout);
	m_orderedDescriptorLayouts.push_back(m_samplerDescriptorLayout);

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

	/*VkDescriptorSetLayoutBinding modelLayoutBinding = {};
	modelLayoutBinding.binding = 1; //Binding number, check in vert shader
	modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	modelLayoutBinding.descriptorCount = 1;
	modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	modelLayoutBinding.pImmutableSamplers = nullptr;*/

	std::vector< VkDescriptorSetLayoutBinding> bindings = { vpLayoutBinding }; //modelLayoutBinding no longer used, using push constant
	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutCreateInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(Vk::Instance().m_device, &layoutCreateInfo, nullptr, &m_descriptorLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout");

	//Texture sampler descriptor set layout
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 0; //Binding number, check in vert shader
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	samplerCreateInfo.bindingCount = 1;
	samplerCreateInfo.pBindings = &samplerLayoutBinding;

	if (vkCreateDescriptorSetLayout(Vk::Instance().m_device, &samplerCreateInfo, nullptr, &m_samplerDescriptorLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout");

}



void Material::CreateDescriptorSets()
{
	m_descriptorSets.resize(Vk::Instance().m_swapChainImages.size());

	std::vector<VkDescriptorSetLayout> setLayouts(Vk::Instance().m_swapChainImages.size(), m_descriptorLayout);

	VkDescriptorSetAllocateInfo setAllocateInfo = {};
	setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocateInfo.descriptorPool = Vk::Instance().m_descriptorPool;
	setAllocateInfo.descriptorSetCount = static_cast<uint32_t>(Vk::Instance().m_swapChainImages.size());
	setAllocateInfo.pSetLayouts = setLayouts.data();

	//Allocate descriptor sets
	if (vkAllocateDescriptorSets(Vk::Instance().m_device, &setAllocateInfo, m_descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor sets");
	}

	for (size_t i = 0; i < Vk::Instance().m_swapChainImages.size(); i++)
	{
		VkDescriptorBufferInfo VPbufferInfo = {};
		VPbufferInfo.buffer = Vk::Instance().m_VPUniformBuffers[i].buffer;
;
		VPbufferInfo.offset = 0;
		VPbufferInfo.range = sizeof(_ViewProjection);

		std::array<VkDescriptorBufferInfo, 1> infos = { VPbufferInfo };
		VkWriteDescriptorSet mvpSetWrite = {};
		mvpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		mvpSetWrite.dstSet = m_descriptorSets[i];
		mvpSetWrite.dstBinding = 0;
		mvpSetWrite.dstArrayElement = 0;
		mvpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		mvpSetWrite.descriptorCount = 1;
		mvpSetWrite.pBufferInfo = infos.data();

		//Model Descriptor

		//binding info
		//Not used, changed to push constant
		/*VkDescriptorBufferInfo modelBufferInfo = {};
		modelBufferInfo.buffer = m_modelDynamicPuniformBuffer[i];
		modelBufferInfo.offset = 0;
		modelBufferInfo.range = m_modelUniformAlignment;

		VkWriteDescriptorSet modelSetWrite = {};
		modelSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		modelSetWrite.dstSet = m_descriptorSets[i];
		modelSetWrite.dstBinding = 1;
		modelSetWrite.dstArrayElement = 0;
		modelSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		modelSetWrite.descriptorCount = 1;
		modelSetWrite.pBufferInfo = &modelBufferInfo;*/

		std::vector< VkWriteDescriptorSet> setWrites = { mvpSetWrite };
		vkUpdateDescriptorSets(Vk::Instance().m_device, setWrites.size(), setWrites.data(), 0, nullptr);
	}
}

int Material::CreateTexture(std::string fileName)
{
	int textureImgaLoc = CreateTextureImage(fileName);
	VkImageView imageView = Vk::Instance().CreateImageView(m_textureImages[textureImgaLoc], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	m_textureImagesViews.push_back(imageView);

	int descriptorLoc = CreateTextureDescriptor(imageView);
	return descriptorLoc;
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
	VkDescriptorSet descriptorSet;

	VkDescriptorSetAllocateInfo setAllocateInfo = {};
	setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocateInfo.descriptorPool = Vk::Instance().m_samplerDescriptorPool;
	setAllocateInfo.descriptorSetCount = 1;
	setAllocateInfo.pSetLayouts = &m_samplerDescriptorLayout;

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
	return m_samplerDescriptorSets.size() - 1;
}