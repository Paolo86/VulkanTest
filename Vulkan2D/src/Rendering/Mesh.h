#pragma once
#include <vulkan/vulkan.h>
#include "Vertex.h"
#include <vector>
#include <vk_mem_alloc.h>
#include "CommonStructs.h"
#include "Material.h"




class Mesh
{
public:
	Mesh(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQ, VkCommandPool transferPool, std::vector<Vertex>& vertices, 
		std::vector<uint32_t>& indices, Material* material);

	Mesh() {};
	~Mesh() {};
	int GetVertexCount();
	VkBuffer GetVertexBuffer();
	VkBuffer GetIndexBuffer();
	uint32_t GetIndexCount() { return m_indexCount; }

	UboModel uboModel;

	void DestroyVertexBuffer();
	Material* material;
private:
	int m_vertexCount;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;

	UniformBuffer<uint32_t> m_IndexBuffer;
	UniformBuffer<Vertex> m_VertexBuffer;
	int m_indexCount;
	
	VkQueue transferQ;
	VkCommandPool transferPool;

	void CreateVertexBuffer(VkQueue transferQ, VkCommandPool transferPool, std::vector<Vertex>& vertices);
	void CreateIndexBuffer(VkQueue transferQ, VkCommandPool transferPool, std::vector<uint32_t>& indices);

};