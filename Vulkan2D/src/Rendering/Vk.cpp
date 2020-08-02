#define NOMINMAX
#include "VkDebugMessanger.h"
#include "Vk.h"
#include "Window.h"
#include "../Utils/Logger.h"
#include <set>
#include <algorithm>
#include <cstdint>
#include "../Utils/FileUtils.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include "Material.h"
#include "VkUtils.h"
#include "../Asset/ResourceManager.h"


std::unique_ptr<Vk> Vk::m_instance;

Material woodMaterial("basic");
Material wallMaterial("basic");
namespace 
{
	const std::vector<const char*> supportedDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	const int MAX_FRAME_DRAWS = 2;
	const int MAX_OBJECTS = 50;
}


Vk& Vk::Instance()
{
	if (m_instance == nullptr)
		m_instance = std::make_unique<Vk>();

	return *m_instance;
}

Vk::Vk()
{

}


Vk::~Vk()
{

}


void Vk::Init()
{
	VkContext::Instance().Init();

	CreateDescriptorPool();
	CreateDepthBufferImage();
	CreateRenderPass();
	CreateFramebuffers();
	//AllocateDynamicBufferTransferSpace(); //Not used, using push constant
	m_textureSampler.Create(
		VkContext::Instance().GetLogicalDevice(),
		VK_FILTER_LINEAR,
		VK_FILTER_LINEAR,
		VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_MIPMAP_MODE_LINEAR,
		0.0, 0.0, false, 16);
	CreateUniformBuffers();

	ViewProjection.projection = glm::perspective(glm::radians(60.0f), (float)VkContext::Instance().GetSwapChainExtent().width / VkContext::Instance().GetSwapChainExtent().height, 0.01f, 1000.0f);
	ViewProjection.view = glm::lookAt(glm::vec3(0, 0, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	ViewProjection.projection[1][1] *= -1;  //Vulkan inverts the Y axis...

	Vertex v1;
	v1.pos = glm::vec3(-0.5, -0.5, 0.0);
	v1.color = glm::vec3(1.0, 0.0, 0.0);
	v1.uvs = glm::vec2(0, 0);

	Vertex v2;
	v2.pos = glm::vec3(0.5, -0.5, 0.0);
	v2.color = glm::vec3(0.0, 1.0, 0.0);
	v2.uvs = glm::vec2(1, 0);

	Vertex v3;
	v3.pos = glm::vec3(0.5, 0.5, 0.0);
	v3.color = glm::vec3(0.0, 0.0, 1.0);
	v3.uvs = glm::vec2(1, 1);

	Vertex v4;
	v4.pos = glm::vec3(-0.5, 0.5, 0.0);
	v4.color = glm::vec3(0.0, 0.0, 1.0);
	v4.uvs = glm::vec2(0, 1);

	std::vector<Vertex> vertices = { v1,v2,v3, v4 };
	std::vector<uint32_t> indices = { 0,1,2,2,3,0 };

	woodMaterial.Create({"wood.jpg"});
	wallMaterial.Create({"wall.jpg"});
	firstMesh = Mesh(VkContext::Instance().GetPhysicalDevice(), VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(), VkContext::Instance().GetCommandPool(), vertices, indices, &woodMaterial);
	firstMesh.uboModel.model = glm::translate(firstMesh.uboModel.model, glm::vec3(0.2, 0, -0.1));

	secondMesh = Mesh(VkContext::Instance().GetPhysicalDevice(), VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(), VkContext::Instance().GetCommandPool(), vertices, indices, &wallMaterial);
	secondMesh.uboModel.model = glm::translate(secondMesh.uboModel.model, glm::vec3(-0.2, 0, -0.1));
	secondMesh.uboModel.model = glm::rotate(secondMesh.uboModel.model, glm::radians(45.0f), glm::vec3(0, 0, 1));

	m_meshes.push_back(firstMesh);
	m_meshes.push_back(secondMesh);


}

void Vk::Destroy()
{
	firstMesh.DestroyVertexBuffer();
	secondMesh.DestroyVertexBuffer();
	vkDeviceWaitIdle(VkContext::Instance().GetLogicalDevice()); //Wait for device to be idle before cleaning up (so won't clean commands currently on queue)

	m_textureSampler.Destroy(VkContext::Instance().GetLogicalDevice());
	vkDestroyDescriptorPool(VkContext::Instance().GetLogicalDevice(), m_descriptorPool, nullptr);
	vkDestroyDescriptorPool(VkContext::Instance().GetLogicalDevice(), m_samplerDescriptorPool, nullptr);

	m_depthBufferImage.Destroy();
	woodMaterial.Destroy();
	wallMaterial.Destroy();
	
	//_aligned_free(m_modelTransferSpace);

	for (auto framebuffer : m_swapChainFramebuffers) {
		vkDestroyFramebuffer(VkContext::Instance().GetLogicalDevice(), framebuffer, nullptr);
	}
	for (size_t i = 0; i < VkContext::Instance().GetSwapChainImagesCount(); i++)
	{
		m_VPUniformBuffers[i].Destroy(VkContext::Instance().GetLogicalDevice());

	}

	vkDestroyRenderPass(VkContext::Instance().GetLogicalDevice(), m_renderPass, nullptr);

	VkContext::Instance().Destroy();
}


void Vk::CreateUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(_ViewProjection);

	//VkDeviceSize modelBufferSize = m_modelUniformAlignment * MAX_OBJECTS;

	m_VPUniformBuffers.resize(VkContext::Instance().GetSwapChainImagesCount());

	//m_modelDynamicPuniformBuffer.resize(m_swapChainImages.size());
	//m_modelDynamicuniformBufferMemory.resize(m_swapChainImages.size());

	for (size_t i = 0; i < VkContext::Instance().GetSwapChainImagesCount(); i++)
	{
		//Vk::Instance().CreateBuffer(modelBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &m_modelDynamicPuniformBuffer[i], &m_modelDynamicuniformBufferMemory[i]);
		m_VPUniformBuffers[i] =  UniformBuffer<_ViewProjection>(VkContext::Instance().GetPhysicalDevice(), VkContext::Instance().GetLogicalDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}
}

void Vk::CreateDescriptorPool()
{
	//Create uniform descriptor pool
	VkDescriptorPoolSize VPpoolSize = {};
	VPpoolSize.descriptorCount = static_cast<uint32_t>(VkContext::Instance().GetSwapChainImagesCount());
	VPpoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	/*VkDescriptorPoolSize modelPoolSize = {};
	modelPoolSize.descriptorCount = static_cast<uint32_t>(m_modelDynamicPuniformBuffer.size());
	modelPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;*/

	std::vector< VkDescriptorPoolSize> poolSizes = { VPpoolSize }; //modelPoolSize no longer used, using push constant

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = static_cast<uint32_t>(VkContext::Instance().GetSwapChainImagesCount() * 2);
	poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolCreateInfo.pPoolSizes = poolSizes.data();

	if (vkCreateDescriptorPool(VkContext::Instance().GetLogicalDevice(), &poolCreateInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool");
	}

	//Create sampler descriptor pool
	VkDescriptorPoolSize samplerPoolSize = {};
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount = 200;		//This is the max number of textures to sample

	VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {};
	samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	samplerPoolCreateInfo.maxSets = 200;
	samplerPoolCreateInfo.poolSizeCount = 1;
	samplerPoolCreateInfo.pPoolSizes = &samplerPoolSize;

	if (vkCreateDescriptorPool(VkContext::Instance().GetLogicalDevice(), &samplerPoolCreateInfo, nullptr, &m_samplerDescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool");
	}
}



void Vk::CreateRenderPass()
{
	// ATTACHMENTS
	// Colour attachment of render pass
	VkAttachmentDescription colourAttachment = {};
	colourAttachment.format = VkContext::Instance().GetSwapChainImageFormat();						// Format to use for attachment
	colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;					// Number of samples to write for multisampling
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;				// Describes what to do with attachment before rendering
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;			// Describes what to do with attachment after rendering
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// Describes what to do with stencil before rendering
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// Describes what to do with stencil after rendering

	// Framebuffer data will be stored as an image, but images can be given different data layouts
	// to give optimal use for certain operations
	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// Image data layout before render pass starts
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		// Image data layout after render pass (to change to)


	// Depth attachment of render pass
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = VkContext::Instance().ChooseSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// REFERENCES
	// Attachment reference uses an attachment index that refers to index in the attachment list passed to renderPassCreateInfo
	VkAttachmentReference colourAttachmentReference = {};
	colourAttachmentReference.attachment = 0;
	colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth Attachment Reference
	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Information about a particular subpass the Render Pass is using
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;		// Pipeline type subpass is to be bound to
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colourAttachmentReference;
	subpass.pDepthStencilAttachment = &depthAttachmentReference;

	// Need to determine when layout transitions occur using subpass dependencies
	std::array<VkSubpassDependency, 2> subpassDependencies;

	// Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// Transition must happen after...
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;						// Subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;		// Pipeline stage
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;				// Stage access mask (memory access)
	// But must happen before...
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;


	// Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// Transition must happen after...
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;;
	// But must happen before...
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;

	std::array<VkAttachmentDescription, 2> renderPassAttachments = { colourAttachment, depthAttachment };

	// Create info for Render Pass
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachments.size());
	renderPassCreateInfo.pAttachments = renderPassAttachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	VkResult result = vkCreateRenderPass(VkContext::Instance().GetLogicalDevice(), &renderPassCreateInfo, nullptr, &m_renderPass);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Render Pass!");
	}
}

void Vk::CreateDepthBufferImage()
{

	m_depthBufferImage = ResourceManager::CreateDepthBufferImage();
}


void  Vk::CreateFramebuffers()
{
	m_swapChainFramebuffers.resize(VkContext::Instance().GetSwapChainImagesCount());

	for (size_t i = 0; i < VkContext::Instance().GetSwapChainImagesCount(); i++) {

		std::array<VkImageView, 2> attachments = {
			VkContext::Instance().GetSwapChainImageViewAt(i) ,
			m_depthBufferImage.m_imageView
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = VkContext::Instance().GetSwapChainExtent().width;
		framebufferInfo.height = VkContext::Instance().GetSwapChainExtent().height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(VkContext::Instance().GetLogicalDevice(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}



void Vk::RenderCmds(uint32_t imageIndex)
{
	VkCommandBufferBeginInfo bufferBeginInfo = {};
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;


	//Info about render pass
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_renderPass;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = VkContext::Instance().GetSwapChainExtent();

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0.6f, 0.65f, 0.4f, 1.0f };
	clearValues[1].depthStencil.depth = 1.0f;

	renderPassBeginInfo.pClearValues = clearValues.data();					// List of clear values
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

	renderPassBeginInfo.framebuffer = m_swapChainFramebuffers[imageIndex];

	//Start recording
	VkResult result = vkBeginCommandBuffer( VkContext::Instance().GetCommandBuferAt(imageIndex), &bufferBeginInfo);
	if (result)
	{
		throw std::runtime_error("Failed to start recording command buffer");
	}

	vkCmdBeginRenderPass(VkContext::Instance().GetCommandBuferAt(imageIndex), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); //All render commands are primary

	//Bind pipeline to be used

	for (size_t j = 0; j < m_meshes.size(); j++)
	{
		VkBuffer vertexBuffer[] = { m_meshes[j].GetVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindPipeline(VkContext::Instance().GetCommandBuferAt(imageIndex), VK_PIPELINE_BIND_POINT_GRAPHICS, m_meshes[j].material->m_graphicsPipeline.m_graphicsPipeline);

		vkCmdBindVertexBuffers(VkContext::Instance().GetCommandBuferAt(imageIndex), 0, 1, vertexBuffer, offsets);
		vkCmdBindIndexBuffer(VkContext::Instance().GetCommandBuferAt(imageIndex), m_meshes[j].GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

		//uint32_t dynamicOffset = static_cast<uint32_t>(m_modelUniformAlignment * j);
		vkCmdPushConstants(VkContext::Instance().GetCommandBuferAt(imageIndex), m_meshes[j].material->m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UboModel), &m_meshes[j].uboModel.model);
		
		std::array<VkDescriptorSet, 2> dsets = { m_meshes[j].material->m_UBOdescriptorSets[imageIndex].m_descriptorSet , m_meshes[j].material->m_samplerDescriptorSets[0].m_descriptorSet };
		
		vkCmdBindDescriptorSets(VkContext::Instance().GetCommandBuferAt(imageIndex), VK_PIPELINE_BIND_POINT_GRAPHICS, m_meshes[j].material->m_pipelineLayout, 0,
			static_cast<uint32_t>(dsets.size()),
			dsets.data(), 0,nullptr);

		vkCmdDrawIndexed(VkContext::Instance().GetCommandBuferAt(imageIndex), m_meshes[j].GetIndexCount(), 1, 0, 0, 0);
	}

	vkCmdEndRenderPass(VkContext::Instance().GetCommandBuferAt(imageIndex));

	//End recording
	result = vkEndCommandBuffer(VkContext::Instance().GetCommandBuferAt(imageIndex));
	if (result)
	{
		throw std::runtime_error("Failed to end recording command buffer");
	}
}



void Vk::Draw()
{
	uint32_t imageIndex;

	VkContext::Instance().WaitForFenceAndAcquireImage(imageIndex);

	RenderCmds(imageIndex);
	m_VPUniformBuffers[imageIndex].Update(VkContext::Instance().GetLogicalDevice(), &ViewProjection);

	VkContext::Instance().Present(imageIndex);
}

void Vk::AllocateDynamicBufferTransferSpace()
{

	m_modelUniformAlignment = (sizeof(UboModel) + VkContext::Instance().GetMinUniformBufferOffset() - 1) & ~(VkContext::Instance().GetMinUniformBufferOffset() - 1);
	//Fixed space ot hold all model matrices of all objects
	m_modelTransferSpace = (UboModel*)_aligned_malloc(m_modelUniformAlignment* MAX_OBJECTS, m_modelUniformAlignment);

}