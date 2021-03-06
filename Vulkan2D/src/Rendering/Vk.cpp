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
#include "..\Core\Timer.h"
#include "..\Lighting\LightManager.h"
#include "..\Core\Components\InstanceRenderer.h"
#include "UBOBatch.h"

#define MESH_COUNT 1
#define USE_BATCHING 1
std::unique_ptr<Vk> Vk::m_instance;

Material woodMaterial("Wood");
Material wallMaterial("Wall");

MeshRenderer meshes[MESH_COUNT];
InstanceRenderer instanceRenderer;
UBOBatch uboBatch;

 namespace
{
	const std::vector<const char*> supportedDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	const int MAX_FRAME_DRAWS = 3;
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
	auto mesh = ResourceManager::LoadModel("Models\\nanosuit.obj","Sphere");

	woodMaterial.Create(ResourceManager::GetPipeline("Basic"),{"wood.jpg"});
	wallMaterial.Create(ResourceManager::GetPipeline("PBR") ,{
		"Iron\\iron_albedo.jpg",
		"Iron\\iron_normal.jpg",
		"Iron\\iron_metallic.jpg",
		"Iron\\iron_roughness.jpg"
		});
	wallMaterial.SetPBRProps(0.0, 0.0, 0);

	for (int i = 0; i < MESH_COUNT; i++)
	{
		meshes[i].SetMesh(ResourceManager::GetMesh("Sphere"));
		meshes[i].SetMaterial(&wallMaterial);

		meshes[i].uboModel.model = glm::translate(meshes[i].uboModel.model, glm::vec3(i * 10,0, -55));
		AddMeshRenderer(&meshes[i], USE_BATCHING);

	}

	m_staticBatch.PrepareStaticBuffers(); //Call after adding all static objects


	//Instance test
	//uboBatch.SetMaterial(&wallMaterial);
	//uboBatch.SetMesh(ResourceManager::GetMesh("Sphere"));

	/*std::vector<InstanceTransform> transforms;
	for (int i = 0; i < MESH_COUNT; i++)
	{
		InstanceTransform t;
		t.position = glm::vec3(i * 10, 0, -50);
		t.rotation = glm::vec3(0, 0, 0);
		t.scale = glm::vec3(1, 1, 1);
		transforms.push_back(t);
	}

	uboBatch.Create(transforms);*/

	//instanceRenderer.Create(transforms);
	LightManager::Instance().Init();
}

void Vk::Destroy()
{

	vkDeviceWaitIdle(VkContext::Instance().GetLogicalDevice()); //Wait for device to be idle before cleaning up (so won't clean commands currently on queue)

	m_textureSampler.Destroy(VkContext::Instance().GetLogicalDevice());
	vkDestroyDescriptorPool(VkContext::Instance().GetLogicalDevice(), m_descriptorPool, nullptr);
	vkDestroyDescriptorPool(VkContext::Instance().GetLogicalDevice(), m_samplerDescriptorPool, nullptr);

	m_staticBatch.DestroyBuffers();

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
	}
}

void Vk::UpdateView(glm::vec3 pos, glm::vec3 dir)
{
	ViewProjection.view = glm::lookAt(pos, dir, glm::vec3(0, 1, 0));
	ViewProjection.camPosition = glm::vec4(pos,1.0);
	for (size_t i = 0; i < VkContext::Instance().GetSwapChainImagesCount(); i++)
	{
		m_VPUniformBuffers[i].Update(VkContext::Instance().GetLogicalDevice(), &ViewProjection);
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
	poolCreateInfo.maxSets = static_cast<uint32_t>(VkContext::Instance().GetSwapChainImagesCount() * 10);
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
	vkBeginCommandBuffer( VkContext::Instance().GetCommandBuferAt(imageIndex), &bufferBeginInfo);

	vkCmdBeginRenderPass(VkContext::Instance().GetCommandBuferAt(imageIndex), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); //All render commands are primary

	VkDeviceSize offsets[] = { 0 };

	//Logger::LogInfo("StartRendering");
	for (auto pipIt = m_allPipelineUsed.begin(); pipIt != m_allPipelineUsed.end(); pipIt++)
	{
		//Bind pipeline
		LightManager::Instance().BindDescriptorSet(VkContext::Instance().GetCommandBuferAt(imageIndex), (*pipIt)->GetPipelineLayout());
		(*pipIt)->Bind(VkContext::Instance().GetCommandBuferAt(imageIndex), imageIndex);

		//Go through the map of dynamic object
		for (auto meshIt = m_dynamicObjrenderMap[(*pipIt)].begin(); meshIt != m_dynamicObjrenderMap[(*pipIt)].end(); meshIt++)
		{
			//Bind mesh buffers
			meshIt->first->BindBuffers(VkContext::Instance().GetCommandBuferAt(imageIndex));
			for (auto matName = meshIt->second.begin(); matName != meshIt->second.end(); matName++)
			{
				matName->first->Bind(VkContext::Instance().GetCommandBuferAt(imageIndex));
				for (auto mr : matName->second)
				{
					vkCmdPushConstants(VkContext::Instance().GetCommandBuferAt(imageIndex), (*pipIt)->GetPipelineLayout(),
						VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UboModel), &mr->uboModel.model);

					vkCmdDrawIndexed(VkContext::Instance().GetCommandBuferAt(imageIndex), mr->m_mesh->GetIndexCount(), 1, 0, 0, 0);
				}		
			}
		}
	}

	//Render all batches using this pipeline, no need to bind it again
	m_staticBatch.RenderBatches(imageIndex);
	/*LightManager::Instance().BindDescriptorSet(VkContext::Instance().GetCommandBuferAt(imageIndex), uboBatch.m_material->GetPipelineUsed()->GetPipelineLayout());

	uboBatch.m_material->GetPipelineUsed()->Bind(VkContext::Instance().GetCommandBuferAt(imageIndex), imageIndex);
	uboBatch.m_material->Bind(VkContext::Instance().GetCommandBuferAt(imageIndex));
	uboBatch.BindBuffers(VkContext::Instance().GetCommandBuferAt(imageIndex));
	uboBatch.Draw(imageIndex);*/

	/*instanceRenderer.m_material->GetPipelineUsed()->Bind(VkContext::Instance().GetCommandBuferAt(imageIndex), imageIndex);
	instanceRenderer.m_material->Bind(VkContext::Instance().GetCommandBuferAt(imageIndex));
	instanceRenderer.BindBuffers(VkContext::Instance().GetCommandBuferAt(imageIndex));
	instanceRenderer.Draw(imageIndex);*/

	vkCmdEndRenderPass(VkContext::Instance().GetCommandBuferAt(imageIndex));

	//End recording
	vkEndCommandBuffer(VkContext::Instance().GetCommandBuferAt(imageIndex));
}



void Vk::Draw()
{
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
		m_staticBatch.AddMeshRenderer(meshRenderer);
	}

	m_allPipelineUsed.insert(meshRenderer->m_material->m_pipeline);
}




