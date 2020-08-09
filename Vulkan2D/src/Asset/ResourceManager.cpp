#include "ResourceManager.h"
#include "..\Rendering\VkUtils.h"
#include "..\Rendering\VkContext.h"

stbi_uc* ResourceManager::LoadTexture(std::string fileName, int* width, int* height, int* channels)
{
	std::string fileLoc = "Textures/" + fileName;
	stbi_uc* image = stbi_load(fileLoc.c_str(), width, height, channels, STBI_rgb_alpha);

	if (!image)
		throw std::runtime_error("Failed to load texture " + fileName);

	return image;
}

Texture2D ResourceManager::CreateSampleTextureFromFile(std::string fileName, VkFormat format)
{
	int w, h, c;
	stbi_uc* imageData = LoadTexture(fileName, &w, &h, &c);

	Texture2D texture;
	VkDeviceSize size = w * h * 4; //Alpha channel is basically forced, so RGBA, 4 channels in total
	VkBuffer staging;
	VkDeviceMemory stagingMemory;
	VkUtils::MemoryUtils::CreateBuffer(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetPhysicalDevice(),
		size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging, &stagingMemory);

	void* data;
	vkMapMemory(VkContext::Instance().GetLogicalDevice(), stagingMemory, 0, size, 0, &data);
	memcpy(data, imageData, static_cast<size_t>(size));
	vkUnmapMemory(VkContext::Instance().GetLogicalDevice(), stagingMemory);

	texture.m_image = VkUtils::ImageUtils::CreateImage(VkContext::Instance().GetPhysicalDevice(), VkContext::Instance().GetLogicalDevice(), w, h,
		format, VK_IMAGE_TILING_OPTIMAL, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture.m_imageMemory);
	//Before copy, transition layout to optimal transfer
	VkUtils::ImageUtils::TransitionImageLayout(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(), VkContext::Instance().GetCommandPool(),
		texture.m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkUtils::ImageUtils::CopyImageBuffer(VkContext::Instance().GetLogicalDevice(), 
		VkContext::Instance().GetGraphicsTransferQ(), VkContext::Instance().GetCommandPool(), staging, texture.m_image, w, h);

	//After copy, transfer layout to shader readable
	VkUtils::ImageUtils::TransitionImageLayout(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(), 
		VkContext::Instance().GetCommandPool(), texture.m_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(VkContext::Instance().GetLogicalDevice(), staging, nullptr);
	vkFreeMemory(VkContext::Instance().GetLogicalDevice(), stagingMemory, nullptr);

	texture.m_imageView = VkUtils::ImageUtils::CreateImageView(VkContext::Instance().GetLogicalDevice(), texture.m_image, format, VK_IMAGE_ASPECT_COLOR_BIT);
	return texture;
}

Texture2D ResourceManager::CreateDepthBufferImage()
{
	Texture2D texture;
	VkFormat depthFormat = VkContext::Instance().ChooseSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	texture.m_image = VkUtils::ImageUtils::CreateImage(VkContext::Instance().GetPhysicalDevice(), VkContext::Instance().GetLogicalDevice(),
		VkContext::Instance().GetSwapChainExtent().width, VkContext::Instance().GetSwapChainExtent().height, depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture.m_imageMemory);

	texture.m_imageView = VkUtils::ImageUtils::CreateImageView(VkContext::Instance().GetLogicalDevice(), texture.m_image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	return texture;

}
