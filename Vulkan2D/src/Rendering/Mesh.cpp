#include "Mesh.h"
#include "Vk.h"

Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQ, VkCommandPool transferPool, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	m_physicalDevice = physicalDevice;
	m_device = device;
	m_vertexCount = vertices.size();
	m_indexCount = indices.size();
	CreateVertexBuffer(transferQ, transferPool, vertices);
	CreateVertexBuffer(transferQ, transferPool, indices);

	uboModel.model = glm::mat4(1);
}

Mesh::Mesh(VmaAllocator& allocator, std::vector<Vertex>& vertices)
{
	this->allocator = &allocator;
	m_vertexCount = vertices.size();

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = sizeof(Vertex) * vertices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

	VmaAllocation allocation;
	vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &m_vertexBuffer, &allocation, nullptr);

	void* mappedData;
	vmaMapMemory(allocator, allocation, &mappedData);
	memcpy(mappedData, vertices.data(), (size_t)bufferInfo.size);
	vmaUnmapMemory(allocator, allocation);

}


int Mesh::GetVertexCount()
{
	return m_vertexCount;
}

VkBuffer Mesh::GetVertexBuffer()
{
	return m_vertexBuffer;
}

VkBuffer Mesh::GetIndexBuffer()
{
	return m_indexBuffer;
}


void Mesh::DestroyVertexBuffer()
{
	vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
	vkFreeMemory(m_device, m_bufferMemory, nullptr);

	vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
	vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
}

void Mesh::CreateVertexBuffer(VkQueue transferQ, VkCommandPool transferPool, std::vector<Vertex>& vertices)
{
	VkDeviceSize size = sizeof(Vertex) * vertices.size();

	//Stage buffer for GPU transfer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	Vk::Instance().CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	//Map memory to vertex buffer
	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, size,0,&data);
	memcpy(data, vertices.data(), (size_t)size);
	vkUnmapMemory(m_device, stagingBufferMemory);

	Vk::Instance().CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_vertexBuffer, &m_bufferMemory);

	Vk::Instance().CopyBuffer(transferQ, transferPool, stagingBuffer, m_vertexBuffer, size);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void Mesh::CreateVertexBuffer(VkQueue transferQ, VkCommandPool transferPool, std::vector<uint32_t>& indices)
{
	VkDeviceSize size = sizeof(uint32_t) * indices.size();

	//Stage buffer for GPU transfer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	Vk::Instance().CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	//Map memory to vertex buffer
	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, size, 0, &data);
	memcpy(data, indices.data(), (size_t)size);
	vkUnmapMemory(m_device, stagingBufferMemory);

	Vk::Instance().CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_indexBuffer, &m_indexBufferMemory);

	Vk::Instance().CopyBuffer(transferQ, transferPool, stagingBuffer, m_indexBuffer, size);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}



