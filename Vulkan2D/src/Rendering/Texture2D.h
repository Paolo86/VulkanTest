#pragma once
#include <vulkan/vulkan.hpp>


class Texture2D
{
public:


	VkImage m_image;
	VkDeviceMemory m_imageMemory;
	VkImageView	m_imageView;
};