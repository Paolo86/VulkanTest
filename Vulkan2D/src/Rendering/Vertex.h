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

	static void GetVertexAttributeDescription(
		VkVertexInputBindingDescription* bindigDescription,
		VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
		std::array<VkVertexInputAttributeDescription, 3>& attributeDescription,
		uint32_t binding = 0);
};