#define NOMINMAX
#include "VkDebugMessanger.h"
#include "Vk.h"
#include "Window.h"
#include "../Utils/Logger.h"
#include <set>
#include <algorithm>
#include <cstdint>
#include "../Utils/FileUtils.h"

#include "Material.h"
#include "VkUtils.h"
#include "../Asset/ResourceManager.h"


std::unique_ptr<Vk> Vk::m_instance;

Material woodMaterial("Wood");
Material wallMaterial("Wall");
MeshRenderer m;
MeshRenderer m2;
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
	VkUtils::MemoryUtils::InitVMA(VkContext::Instance().GetPhysicalDevice(), VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetVkInstance());

	CreateDescriptorPool();
	m_depthBufferImage = ResourceManager::CreateDepthBufferImage();
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

	ViewProjection.projection = glm::perspective(glm::radians(60.0f), (float)VkContext::Instance().GetSwapChainExtent().width / VkContext::Instance().GetSwapChainExtent().height, 0.01f, 1000.0f);
	ViewProjection.view = glm::lookAt(glm::vec3(0, 0, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	ViewProjection.projection[1][1] *= -1;  //Vulkan inverts the Y axis...

	CreateUniformBuffers();
	ResourceManager::CreatePipelines();
	ResourceManager::CreateMeshes();

	woodMaterial.Create(ResourceManager::GetPipeline("Basic"),{"wood.jpg"});
	woodMaterial.SetTint(1, 0, 0, 0.2);
	wallMaterial.Create(ResourceManager::GetPipeline("Basic") ,{"wall.jpg"});


	m.SetMesh(ResourceManager::GetMesh("Quad"));
	m2.SetMesh(ResourceManager::GetMesh("Quad"));
	m.SetMaterial(&woodMaterial);
	m2.SetMaterial(&woodMaterial);
	m.uboModel.model = glm::translate(m.uboModel.model, glm::vec3(-1, 0, 0));
	m2.uboModel.model = glm::translate(m2.uboModel.model, glm::vec3(0, 0, 0));

	AddMeshRenderer(&m,true);
	AddMeshRenderer(&m2,true);
	PrepareStaticBuffers();
}

void Vk::Destroy()
{

	vkDeviceWaitIdle(VkContext::Instance().GetLogicalDevice()); //Wait for device to be idle before cleaning up (so won't clean commands currently on queue)

	m_textureSampler.Destroy(VkContext::Instance().GetLogicalDevice());
	vkDestroyDescriptorPool(VkContext::Instance().GetLogicalDevice(), m_descriptorPool, nullptr);
	vkDestroyDescriptorPool(VkContext::Instance().GetLogicalDevice(), m_samplerDescriptorPool, nullptr);

	m_depthBufferImage.Destroy();
	woodMaterial.Destroy();
	wallMaterial.Destroy();
	
	ResourceManager::DestroyAll();

	for (auto framebuffer : m_swapChainFramebuffers) {
		framebuffer.Destroy(VkContext::Instance().GetLogicalDevice());
	}
	for (size_t i = 0; i < VkContext::Instance().GetSwapChainImagesCount(); i++)
	{
		m_VPUniformBuffers[i].Destroy();

	}

	vkDestroyRenderPass(VkContext::Instance().GetLogicalDevice(), m_renderPass, nullptr);

	VkContext::Instance().Destroy();
}


void Vk::CreateUniformBuffers()
{


	m_VPUniformBuffers.resize(VkContext::Instance().GetSwapChainImagesCount());
	m_dynamicBuffer.resize(VkContext::Instance().GetSwapChainImagesCount());

	for (size_t i = 0; i < VkContext::Instance().GetSwapChainImagesCount(); i++)
	{
		m_VPUniformBuffers[i] =  UniformBuffer<_ViewProjection>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		m_VPUniformBuffers[i].Update(VkContext::Instance().GetLogicalDevice(), &ViewProjection);

		m_dynamicBuffer[i] = DynamicUniformBuffer<_ViewProjection>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);


	}


}

void Vk::CreateDescriptorPool()
{
	//Create uniform descriptor pool
	VkDescriptorPoolSize VPpoolSize = {};
	VPpoolSize.descriptorCount = static_cast<uint32_t>(VkContext::Instance().GetSwapChainImagesCount());
	VPpoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	VkDescriptorPoolSize dynamicBuffer = {};
	dynamicBuffer.descriptorCount = static_cast<uint32_t>(VkContext::Instance().GetSwapChainImagesCount());
	dynamicBuffer.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

	std::vector< VkDescriptorPoolSize> poolSizes = { VPpoolSize }; //modelPoolSize no longer used, using push constant

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = static_cast<uint32_t>(VkContext::Instance().GetSwapChainImagesCount());
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
	samplerPoolCreateInfo.maxSets = 200; //Max number of sets (in this case, number of materials)
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

void  Vk::CreateFramebuffers()
{
	m_swapChainFramebuffers.resize(VkContext::Instance().GetSwapChainImagesCount());

	for (size_t i = 0; i < VkContext::Instance().GetSwapChainImagesCount(); i++) {

		std::array<VkImageView, 2> attachments = {
			VkContext::Instance().GetSwapChainImageViewAt(i) ,
			m_depthBufferImage.m_imageView
		};

		m_swapChainFramebuffers[i].Create(VkContext::Instance().GetLogicalDevice(),
			m_renderPass,
			VkContext::Instance().GetSwapChainExtent().width,
			VkContext::Instance().GetSwapChainExtent().height,
			{ VkContext::Instance().GetSwapChainImageViewAt(i) , m_depthBufferImage.m_imageView });
		
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
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil.depth = 1.0f;

	renderPassBeginInfo.pClearValues = clearValues.data();					// List of clear values
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

	renderPassBeginInfo.framebuffer = m_swapChainFramebuffers[imageIndex].GetVkFrameBuffer();

	//Start recording
	VkResult result = vkBeginCommandBuffer( VkContext::Instance().GetCommandBuferAt(imageIndex), &bufferBeginInfo);
	if (result)
	{
		throw std::runtime_error("Failed to start recording command buffer");
	}

	vkCmdBeginRenderPass(VkContext::Instance().GetCommandBuferAt(imageIndex), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); //All render commands are primary

	VkDeviceSize offsets[] = { 0 };

	Logger::LogInfo("StartRendering");

	for (auto pipIt = m_dynamicObjrenderMap.begin(); pipIt != m_dynamicObjrenderMap.end(); pipIt++)
	{
		//Bind pipeline
		pipIt->first->Bind(VkContext::Instance().GetCommandBuferAt(imageIndex), imageIndex);

		for (auto meshIt = pipIt->second.begin(); meshIt != pipIt->second.end(); meshIt++)
		{
			//Bind mesh buffers
			meshIt->first->BindBuffers(VkContext::Instance().GetCommandBuferAt(imageIndex));
			for (auto matName = meshIt->second.begin(); matName != meshIt->second.end(); matName++)
			{
				matName->first->Bind(VkContext::Instance().GetCommandBuferAt(imageIndex));
				for (auto mr : matName->second)
				{
					vkCmdPushConstants(VkContext::Instance().GetCommandBuferAt(imageIndex), pipIt->first->m_pipelineLayout,
						VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UboModel), &mr->uboModel.model);


					vkCmdDrawIndexed(VkContext::Instance().GetCommandBuferAt(imageIndex), mr->m_mesh->GetIndexCount(), 1, 0, 0, 0);
				}
		
			}
		}

	}

	//TODO do not re record this every time, record cmd once
	for (auto pipIt = m_vertexBuffers.begin(); pipIt != m_vertexBuffers.end(); pipIt++)
	{
		UboModel identity;
		identity.model = glm::mat4(1);

		vkCmdPushConstants(VkContext::Instance().GetCommandBuferAt(imageIndex), pipIt->first->m_pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UboModel), &identity);
		pipIt->first->Bind(VkContext::Instance().GetCommandBuferAt(imageIndex), imageIndex);

		for (auto material = pipIt->second.begin(); material != pipIt->second.end(); material++)
		{
			material->first->Bind(VkContext::Instance().GetCommandBuferAt(imageIndex));

			vkCmdBindVertexBuffers(VkContext::Instance().GetCommandBuferAt(imageIndex), 0, 1, &material->second.buffer, offsets);
			vkCmdBindIndexBuffer(VkContext::Instance().GetCommandBuferAt(imageIndex), m_indexBuffers[pipIt->first][material->first].buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(VkContext::Instance().GetCommandBuferAt(imageIndex), 
				m_indexBuffersCount[pipIt->first][material->first], 1, 0, 0, 0);


		}

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
	static float angle = 0;
	angle = 0.1;
	m.uboModel.model = glm::rotate(m.uboModel.model, glm::radians(angle), glm::vec3(0, 1, 0));

	uint32_t imageIndex;

	VkContext::Instance().WaitForFenceAndAcquireImage(imageIndex);

	RenderCmds(imageIndex);

	VkContext::Instance().Present(imageIndex);
}


void Vk::AddMeshRenderer(MeshRenderer* meshRenderer, bool isStatic)
{
	if(!isStatic)
		m_dynamicObjrenderMap[meshRenderer->m_material->m_pipeline][meshRenderer->m_mesh][meshRenderer->m_material].insert(meshRenderer);
	else
	{
		m_staticObjrenderMap[meshRenderer->m_material->m_pipeline][meshRenderer->m_material].push_back(meshRenderer);
	}
}

void Vk::PrepareStaticBuffers()
{
	for (auto pipeline = m_staticObjrenderMap.begin(); pipeline != m_staticObjrenderMap.end(); pipeline++)
	{
		for (auto material = pipeline->second.begin(); material != pipeline->second.end(); material++)
		{
			uint32_t vertexCount = 0;
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			int vertexOffset = 0;

			for (int mesh = 0; mesh < material->second.size(); mesh++)
			{
				for (int v = 0; v < material->second[mesh]->m_mesh->GetVertexCount(); v++)
				{
					Vertex vertex = material->second[mesh]->m_mesh->GetVertices()[v];
					vertex.pos = material->second[mesh]->uboModel.model * glm::vec4(vertex.pos, 1.0);
					vertices.push_back(vertex);
				}

				for (int v = 0; v < material->second[mesh]->m_mesh->GetIndexCount(); v++)
				{
					indices.push_back(material->second[mesh]->m_mesh->GetIndices()[v] + vertexOffset);
				}

				vertexOffset += material->second[mesh]->m_mesh->GetVertexCount();

				m_indexBuffersCount[pipeline->first][material->first] = indices.size();
			}

			{
				UniformBuffer<Vertex> staging(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY, vertices.size());
				staging.Update(VkContext::Instance().GetLogicalDevice(), vertices.data());

				m_vertexBuffers[pipeline->first][material->first].Create(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VMA_MEMORY_USAGE_GPU_ONLY, vertices.size());
				VkUtils::MemoryUtils::CopyBuffer(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(),
					VkContext::Instance().GetCommandPool(), staging.buffer, m_vertexBuffers[pipeline->first][material->first].buffer, staging.bufferSize);
				staging.Destroy();
			}

			{
				UniformBuffer<uint32_t> stage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, static_cast<uint32_t>(indices.size()));
				stage.Update(VkContext::Instance().GetLogicalDevice(), indices.data());
				m_indexBuffers[pipeline->first][material->first].Create(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, indices.size());
				VkUtils::MemoryUtils::CopyBuffer(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(), 
					VkContext::Instance().GetCommandPool(), stage.buffer, m_indexBuffers[pipeline->first][material->first].buffer, stage.bufferSize);
				stage.Destroy();
			}


		}
	}
}

