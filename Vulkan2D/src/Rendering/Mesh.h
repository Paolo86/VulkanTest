#pragma once
#include <vulkan/vulkan.h>
#include "Vertex.h"
#include <vector>


class Mesh
{
public:
	Mesh(VkPhysicalDevice physicalDevice, VkDevice device, std::vector<Vertex>& vertices);
	Mesh() {};
	~Mesh() {};
	int GetVertexCount();
	VkBuffer GetVertexBuffer();

	void DestroyVertexBuffer();
private:
	int m_vertexCount;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_bufferMemory;

	void CreateVertexBuffer(std::vector<Vertex>& vertices);
	uint32_t FindMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags flags);

};