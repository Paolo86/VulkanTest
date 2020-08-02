#include "Texture2D.h"
#include "VkUtils.h"
#include "Vk.h"

Texture2D::Texture2D(uint32_t width, uint32_t height, uint32_t channels, VkFormat format, void* imageData)
{
	m_format = format;
	VkDeviceSize size = width * height * channels;
	VkBuffer staging;
	VkDeviceMemory stagingMemory;
	VkUtils::MemoryUtils::CreateBuffer(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetPhysicalDevice(),
		size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging, &stagingMemory);

	void* data;
	vkMapMemory(VkContext::Instance().GetLogicalDevice(), stagingMemory, 0, size, 0, &data);
	memcpy(data, imageData, static_cast<size_t>(size));
	vkUnmapMemory(VkContext::Instance().GetLogicalDevice(), stagingMemory);

	m_image = VkUtils::ImageUtils::CreateImage(VkContext::Instance().GetPhysicalDevice(), VkContext::Instance().GetLogicalDevice(),width, height, 
		format, VK_IMAGE_TILING_OPTIMAL, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_imageMemory);
	//Before copy, transition layout to optimal transfer
	VkUtils::ImageUtils::TransitionImageLayout(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(), VkContext::Instance().GetCommandPool(),
		m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkUtils::ImageUtils::CopyImageBuffer(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(), VkContext::Instance().GetCommandPool(), staging, m_image, width, height);

	//After copy, transfer layout to shader readable
	VkUtils::ImageUtils::TransitionImageLayout(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(), VkContext::Instance().GetCommandPool(), m_image, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(VkContext::Instance().GetLogicalDevice(), staging, nullptr);
	vkFreeMemory(VkContext::Instance().GetLogicalDevice(), stagingMemory, nullptr);

	m_imageView = VkUtils::ImageUtils::CreateImageView(VkContext::Instance().GetLogicalDevice(), m_image, format, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Texture2D::Destroy()
{
	vkDestroyImageView(VkContext::Instance().GetLogicalDevice(), m_imageView, nullptr);
	vkDestroyImage(VkContext::Instance().GetLogicalDevice(), m_image, nullptr);
	vkFreeMemory(VkContext::Instance().GetLogicalDevice(), m_imageMemory, nullptr);
}
