#pragma once
#include <vulkan/vulkan.h>
#include "Vertex.h"
#include <vector>
#include <vk_mem_alloc.h>


struct UboModel {
	glm::mat4 model;
};

class Mesh
{
public:
	Mesh(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQ, VkCommandPool transferPool, std::vector<Vertex>& vertices, 
		std::vector<uint32_t>& indices);
	Mesh(VmaAllocator& allocator, std::vector<Vertex>& vertices);

	Mesh() {};
	~Mesh() {};
	int GetVertexCount();
	VkBuffer GetVertexBuffer();
	VkBuffer GetIndexBuffer();
	uint32_t GetIndexCount() { return m_indexCount; }

	UboModel uboModel;

	void DestroyVertexBuffer();
private:
	int m_vertexCount;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_bufferMemory;

	int m_indexCount;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;

	VmaAllocation allocation;

	VkQueue transferQ;
	VkCommandPool transferPool;

	VmaAllocator* allocator;
	void CreateVertexBuffer(VkQueue transferQ, VkCommandPool transferPool, std::vector<Vertex>& vertices);
	void CreateVertexBuffer(VkQueue transferQ, VkCommandPool transferPool, std::vector<uint32_t>& indices);

};