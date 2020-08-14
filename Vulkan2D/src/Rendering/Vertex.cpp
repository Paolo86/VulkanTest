#include "Vertex.h"

void Vertex::GetVertexAttributeDescription(
	VkVertexInputBindingDescription* bindigDescription,
	VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
	std::array<VkVertexInputAttributeDescription, 4>& attributeDescription,
	uint32_t binding)
{
	// Vertex input
	//Data for single vertex (including position, color, normals and so on)
	bindigDescription->binding = 0;
	bindigDescription->stride = sizeof(Vertex);
	bindigDescription->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;		// Move onto next vertex
																	// VK_VERTEX_INPUT_RATE_INSTANCE: move to vertex of next instance
	// Data within the vertex
	
	attributeDescription[0].binding = 0;
	attributeDescription[0].location = 0;
	attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescription[0].offset = offsetof(Vertex, pos);

	attributeDescription[1].binding = 0;
	attributeDescription[1].location = 1;
	attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescription[1].offset = offsetof(Vertex, color);

	attributeDescription[2].binding = 0;
	attributeDescription[2].location = 2;
	attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescription[2].offset = offsetof(Vertex, uvs);

	attributeDescription[3].binding = 0;
	attributeDescription[3].location = 3;
	attributeDescription[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescription[3].offset = offsetof(Vertex, normal);

	vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo->vertexBindingDescriptionCount = 1;
	vertexInputInfo->pVertexBindingDescriptions = bindigDescription; // Optional
	vertexInputInfo->vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInputInfo->pVertexAttributeDescriptions = attributeDescription.data(); // Optional
}
