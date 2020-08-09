#include "Mesh.h"
#include "Vk.h"

Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQ, VkCommandPool transferPool, 
	std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Material* material)
{
	m_physicalDevice = physicalDevice;
	m_device = device;
	m_vertexCount = vertices.size();
	m_indexCount = indices.size();
	CreateVertexBuffer(transferQ, transferPool, vertices);
	CreateIndexBuffer(transferQ, transferPool, indices);

	uboModel.model = glm::mat4(1);
	this->material = material;
}

int Mesh::GetVertexCount()
{
	return m_vertexCount;
}

VkBuffer Mesh::GetVertexBuffer()
{
	return m_VertexBuffer.buffer;
}

VkBuffer Mesh::GetIndexBuffer()
{
	return m_IndexBuffer.buffer;
}


void Mesh::DestroyVertexBuffer()
{
	m_VertexBuffer.Destroy(m_device);
	
	m_IndexBuffer.Destroy(m_device);
}

void Mesh::CreateVertexBuffer(VkQueue transferQ, VkCommandPool transferPool, std::vector<Vertex>& vertices)
{

	UniformBuffer<Vertex> staging(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY, vertices.size());
	staging.Update(m_device, vertices.data());
	m_VertexBuffer.Create(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, vertices.size());
	VkUtils::MemoryUtils::CopyBuffer(VkContext::Instance().GetLogicalDevice(), transferQ, transferPool, staging.buffer, m_VertexBuffer.buffer, staging.bufferSize);
	staging.Destroy(m_device);
}

void Mesh::CreateIndexBuffer(VkQueue transferQ, VkCommandPool transferPool, std::vector<uint32_t>& indices)
{
	UniformBuffer<uint32_t> stage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, indices.size());
	stage.Update(m_device, indices.data());
	m_IndexBuffer.Create(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, indices.size());
	VkUtils::MemoryUtils::CopyBuffer(VkContext::Instance().GetLogicalDevice(), transferQ, transferPool, stage.buffer, m_IndexBuffer.buffer, stage.bufferSize);
	stage.Destroy(m_device);
}



