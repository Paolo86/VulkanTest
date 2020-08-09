#pragma once
#include <vulkan/vulkan.h>
#include "Vertex.h"
#include <vector>
#include <vk_mem_alloc.h>
#include "CommonStructs.h"
#include "UniformBuffer.h"



class Mesh
{
public:
	Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

	Mesh() {};
	~Mesh() {
	

	};
	int GetVertexCount();
	VkBuffer GetVertexBuffer();
	VkBuffer GetIndexBuffer();
	uint32_t GetIndexCount() { return m_indexCount; }
	void BindBuffers(VkCommandBuffer cmdBuffer);

	std::vector<Vertex>& GetVertices() { return m_vertices; }
	std::vector<uint32_t>& GetIndices() { return m_indices; }

	void DestroyVertexBuffer();
private:
	int m_vertexCount;

	UniformBuffer<uint32_t> m_IndexBuffer;
	UniformBuffer<Vertex> m_VertexBuffer;
	int m_indexCount;

	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

	void CreateVertexBuffer(std::vector<Vertex>& vertices);
	void CreateIndexBuffer( std::vector<uint32_t>& indices);

};