#include "InstanceRenderer.h"
#include "..\..\Rendering\VkContext.h"
InstanceRenderer::InstanceRenderer() {}

InstanceRenderer::InstanceRenderer(Mesh* mesh, Material* material, std::vector<InstanceTransform> transforms) : m_instanceTransforms(transforms)
{
	this->m_mesh = mesh;
	this->m_material = material;

	UniformBuffer<InstanceTransform> staging(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY, transforms.size());
	staging.Update(VkContext::Instance().GetLogicalDevice(), transforms.data());
	m_instanceTransformsBuffer.Create(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, transforms.size());
	VkUtils::MemoryUtils::CopyBuffer(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(),
		VkContext::Instance().GetCommandPool(), staging.buffer, m_instanceTransformsBuffer.buffer, staging.bufferSize);
	staging.Destroy();
}

void InstanceRenderer::BindBuffers(VkCommandBuffer cmdBuffer)
{
	VkDeviceSize offsets[] = { 0 };
	m_mesh->BindBuffers(cmdBuffer);

	vkCmdBindVertexBuffers(cmdBuffer, 1, 1, &m_instanceTransformsBuffer.buffer, offsets);

}

void InstanceRenderer::Create(std::vector<InstanceTransform> transforms)
{
	m_instanceTransforms = transforms;
	UniformBuffer<InstanceTransform> staging(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY, transforms.size());
	staging.Update(VkContext::Instance().GetLogicalDevice(), transforms.data());
	m_instanceTransformsBuffer.Create(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, transforms.size());
	VkUtils::MemoryUtils::CopyBuffer(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(),
		VkContext::Instance().GetCommandPool(), staging.buffer, m_instanceTransformsBuffer.buffer, staging.bufferSize);
	staging.Destroy();
}


void InstanceRenderer::SetMesh(Mesh* mesh)
{
	this->m_mesh = mesh;

}

void InstanceRenderer::SetMaterial(Material* mat)
{
	this->m_material = mat;

}

void InstanceRenderer::Draw(int imageIndex)
{
	vkCmdDrawIndexed(VkContext::Instance().GetCommandBuferAt(imageIndex), m_mesh->GetIndexCount(), m_instanceTransforms.size(), 0, 0, 0);

}
