#pragma once
#include <vulkan/vulkan.hpp>


class TextureSampler
{
public:
	TextureSampler() {};
	void Create(VkDevice device,
		VkFilter mag,
		VkFilter min,
		VkBorderColor borderColor,
		VkSamplerAddressMode addressModeU,
		VkSamplerAddressMode addressModeV,
		VkSamplerAddressMode addressModeW,
		VkSamplerMipmapMode mipMapMode,
		float minLod,
		float maxLod,
		VkBool32 anisotropyEnabled,
		float anisotropyLevel)
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = mag;
		samplerInfo.minFilter = min;
		samplerInfo.borderColor = borderColor;
		samplerInfo.addressModeU = addressModeU;
		samplerInfo.addressModeV = addressModeV;
		samplerInfo.addressModeW = addressModeW;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.mipmapMode = mipMapMode;
		samplerInfo.minLod = minLod;
		samplerInfo.maxLod = maxLod;
		samplerInfo.anisotropyEnable = anisotropyEnabled;
		samplerInfo.maxAnisotropy = anisotropyLevel;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS)
			throw std::runtime_error("Failed to create sampler");
	}

	void Destroy(VkDevice device)
	{
		vkDestroySampler(device, m_sampler, nullptr);
	}

	VkSampler m_sampler;
};