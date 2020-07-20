#include "Mesh.h"


Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice device, std::vector<Vertex>& vertices)
{
	m_physicalDevice = physicalDevice;
	m_device = device;
	m_vertexCount = vertices.size();

	CreateVertexBuffer(vertices);
}

int Mesh::GetVertexCount()
{
	return m_vertexCount;
}

VkBuffer Mesh::GetVertexBuffer()
{
	return m_vertexBuffer;
}

void Mesh::DestroyVertexBuffer()
{
	vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
	vkFreeMemory(m_device, m_bufferMemory, nullptr);
}

void Mesh::CreateVertexBuffer(std::vector<Vertex>& vertices)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(Vertex) * vertices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult res = vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_vertexBuffer);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vertex buffer");
	}

	VkMemoryRequirements memRequirement = {};
	vkGetBufferMemoryRequirements(m_device, m_vertexBuffer, &memRequirement);

	//Allocate memory
	VkMemoryAllocateInfo memoryAllocInfo= {};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = memRequirement.size;
	memoryAllocInfo.memoryTypeIndex = FindMemoryTypeIndex(memRequirement.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);//VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT: CPU can interact with memory
																					//VK_MEMORY_PROPERTY_HOST_COHERENT_BIT: Place data straight into buffer after mapping


	//Allocate memory to vk device memory
	res = vkAllocateMemory(m_device, &memoryAllocInfo, nullptr, &m_bufferMemory);
	vkBindBufferMemory(m_device, m_vertexBuffer, m_bufferMemory, 0);

	//Map memory to vertex buffer
	void* data;
	vkMapMemory(m_device, m_bufferMemory, 0, bufferInfo.size ,0,&data);
	memcpy(data, vertices.data(), (size_t)bufferInfo.size);
	vkUnmapMemory(m_device, m_bufferMemory);
}

uint32_t Mesh::FindMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags flags)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((allowedTypes & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & flags) == flags) //Check if it's allowed type and all flags are ok
		{
			return i; //We want the index of the memory block
		}
	}
}
