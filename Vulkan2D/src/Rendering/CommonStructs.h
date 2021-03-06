#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>


typedef struct 
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value(); //If queues have been assigned
	}
}QueueFamilyIndices;

// Collect info about swapchain
typedef struct 
{
	VkSurfaceCapabilitiesKHR capabilities; //Resolution of images in swapchain
	std::vector<VkSurfaceFormatKHR> formats; //Color depth
	std::vector<VkPresentModeKHR> presentModes; //Condition for swapping images
}SwapChainSupportDetails;


struct _ViewProjection {

	glm::mat4 projection;
	glm::mat4 view;
	glm::vec4 camPosition;

};

typedef struct {
	glm::mat4 model;
} UboModel;

typedef struct {
	glm::vec4 tint;
	glm::vec4 uvScale;
	glm::vec4 pbrProps;
} MaterialProps;

struct InstanceTransform
{
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};