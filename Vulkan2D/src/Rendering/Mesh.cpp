#include "Mesh.h"
#include "Vk.h"
#include "..\Utils\Logger.h"
Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) : m_vertices(vertices), m_indices(indices)
{
	m_vertexCount = m_vertices.size();
	m_indexCount = m_indices.size();
	CalculateNormals();
	CreateVertexBuffer(m_vertices);
	CreateIndexBuffer(m_indices);

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
	m_VertexBuffer.Destroy();
	
	m_IndexBuffer.Destroy();
}

void Mesh::CreateVertexBuffer(std::vector<Vertex>& m_vertices)
{

	UniformBuffer<Vertex> staging(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY, m_vertices.size());
	staging.Update(VkContext::Instance().GetLogicalDevice(), m_vertices.data());
	m_VertexBuffer.Create(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_vertices.size());
	VkUtils::MemoryUtils::CopyBuffer(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(), 
		VkContext::Instance().GetCommandPool(), staging.buffer, m_VertexBuffer.buffer, staging.bufferSize);
	staging.Destroy();
}

void Mesh::CreateIndexBuffer(std::vector<uint32_t>& m_indices)
{
	UniformBuffer<uint32_t> stage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, static_cast<uint32_t>(m_indices.size()));
	stage.Update(VkContext::Instance().GetLogicalDevice(), m_indices.data());
	m_IndexBuffer.Create(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_indices.size());
	VkUtils::MemoryUtils::CopyBuffer(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(), VkContext::Instance().GetCommandPool(), stage.buffer, m_IndexBuffer.buffer, stage.bufferSize);
	stage.Destroy();
}

void Mesh::BindBuffers(VkCommandBuffer cmdBuffer)
{
	VkDeviceSize offsets[] = { 0 };

	//Logger::LogInfo("\tBind Mesh buffers");
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &m_VertexBuffer.buffer, offsets);
	vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
}

void Mesh::CalculateNormals()
{
	if (m_indices.size() > 3)
	{

		for (unsigned int i = 0; i < m_indices.size() - 2; i += 3)
		{
			//Normals
			glm::vec3 v1v2 = m_vertices[m_indices[i + 1]].pos - m_vertices[m_indices[i]].pos;
			glm::vec3 v1v3 = m_vertices[m_indices[i + 2]].pos - m_vertices[m_indices[i]].pos;
			glm::vec3 cross = glm::normalize(glm::cross(v1v2, v1v3));


			m_vertices[m_indices[i]].normal = cross;
			m_vertices[m_indices[i + 1]].normal = cross;
			m_vertices[m_indices[i + 2]].normal = cross;

			//Tangent and binormal
			glm::vec2 deltaUV1 = m_vertices[m_indices[i + 1]].uvs - m_vertices[m_indices[i]].uvs;
			glm::vec2 deltaUV2 = m_vertices[m_indices[i + 2]].uvs - m_vertices[m_indices[i]].uvs;

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

			glm::vec3 test = r * ((v1v2 * deltaUV2.y) - (v1v3 * deltaUV1.y));

			m_vertices[m_indices[i]].tangent = r * ((v1v2 * deltaUV2.y) - (v1v3 * deltaUV1.y));
			m_vertices[m_indices[i + 1]].tangent = r * ((v1v2 * deltaUV2.y) - (v1v3 * deltaUV1.y));
			m_vertices[m_indices[i + 2]].tangent = r * ((v1v2 * deltaUV2.y) - (v1v3 * deltaUV1.y));

			m_vertices[m_indices[i]].binormal = r * ((v1v3 * deltaUV1.x) - (v1v2 * deltaUV2.x));
			m_vertices[m_indices[i + 1]].binormal = r * ((v1v3 * deltaUV1.x) - (v1v2 * deltaUV2.x));
			m_vertices[m_indices[i + 2]].binormal = r * ((v1v3 * deltaUV1.x) - (v1v2 * deltaUV2.x));

		}
	}
}



