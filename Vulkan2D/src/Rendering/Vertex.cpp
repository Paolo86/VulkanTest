#include "Vertex.h"
#include "CommonStructs.h"

void Vertex::GetVertexAttributeDescription(
	std::vector<VkVertexInputBindingDescription>& bindigDescription,
	VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
	std::vector< VkVertexInputAttributeDescription>& attributeDescription,
	bool setUpInstancing)
{

	if (setUpInstancing)
	{
		bindigDescription.resize(2);
		attributeDescription.resize(9);
	}
	else
	{
		bindigDescription.resize(1);
		attributeDescription.resize(6);
	}

	// Vertex input
	//Data for single vertex (including position, color, normals and so on)
	bindigDescription[0].binding = 0;
	bindigDescription[0].stride = sizeof(Vertex);
	bindigDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	if (setUpInstancing)
	{
		bindigDescription[1].binding = 1;
		bindigDescription[1].stride = sizeof(InstanceTransform);
		bindigDescription[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
	}
																	
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

	attributeDescription[4].binding = 0;
	attributeDescription[4].location = 4;
	attributeDescription[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescription[4].offset = offsetof(Vertex, tangent);

	attributeDescription[5].binding = 0;
	attributeDescription[5].location = 5;
	attributeDescription[5].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescription[5].offset = offsetof(Vertex, binormal);

	if (setUpInstancing)
	{
 		attributeDescription[6].binding = 1;
		attributeDescription[6].location = 6;
		attributeDescription[6].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[6].offset = offsetof(InstanceTransform, position);

		attributeDescription[7].binding = 1;
		attributeDescription[7].location = 7;
		attributeDescription[7].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[7].offset = offsetof(InstanceTransform, rotation);

		attributeDescription[8].binding = 1;
		attributeDescription[8].location = 8;
		attributeDescription[8].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription[8].offset = offsetof(InstanceTransform, scale);
	}

	vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo->vertexBindingDescriptionCount = bindigDescription.size();
	vertexInputInfo->pVertexBindingDescriptions = bindigDescription.data(); // Optional
	vertexInputInfo->vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInputInfo->pVertexAttributeDescriptions = attributeDescription.data(); // Optional
}
