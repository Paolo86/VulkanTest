#pragma once
#include <glm/glm.hpp>
#include <array>
#include <vulkan/vulkan.hpp>



class Vertex
{
public:
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 uvs;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 binormal;
	glm::mat3 TBN;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && uvs == other.uvs && normal == other.normal;
	}

	static void GetVertexAttributeDescription(
		VkVertexInputBindingDescription* bindigDescription,
		VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
		std::array<VkVertexInputAttributeDescription, 6>& attributeDescription,
		uint32_t binding = 0);
};