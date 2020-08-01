#pragma once
#include <vulkan/vulkan.hpp>


class Texture2D
{
public:
	Texture2D(uint32_t width, uint32_t height, uint32_t channels, VkFormat format, void* imageData);

	void Destroy();

	VkImage m_image;
	VkDeviceMemory m_imageMemory;
	VkImageView	m_imageView;
	VkFormat m_format;
};