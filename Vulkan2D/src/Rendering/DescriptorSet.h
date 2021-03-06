#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <array>
#include "UniformBuffer.h"
#include "DynamicUniformBuffer.h"
#include "Texture2D.h"


struct DescriptorSetLayout
{
	DescriptorSetLayout& AddBinding(std::vector<VkDescriptorSetLayoutBinding> bindings)
	{
		for(VkDescriptorSetLayoutBinding binding: bindings)
			m_bindings.push_back(binding);

		return *this;
	}

	void Create(VkDevice device)
	{
		VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCreateInfo.bindingCount = static_cast<uint32_t>(m_bindings.size());
		layoutCreateInfo.pBindings = m_bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &m_descriptorLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create descriptor set layout");
	}

	void Destroy(VkDevice device)
	{
		vkDestroyDescriptorSetLayout(device, m_descriptorLayout, nullptr);
	}
	VkDescriptorSetLayout m_descriptorLayout;
	uint32_t setNumber;

private:
	std::vector< VkDescriptorSetLayoutBinding> m_bindings;

};

class DescriptorSet
{
public:
	DescriptorSet()
	{
	};

	void CreateDescriptorSet(VkDevice device, DescriptorSetLayout layout, VkDescriptorPool pool)
	{		
		VkDescriptorSetAllocateInfo setAllocateInfo = {};
		setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		setAllocateInfo.descriptorPool = pool;
		setAllocateInfo.descriptorSetCount = 1;
		setAllocateInfo.pSetLayouts = &layout.m_descriptorLayout;
		//Allocate descriptor sets
		if (vkAllocateDescriptorSets(device, &setAllocateInfo, &m_descriptorSet) != VK_SUCCESS)
		{
			Logger::LogError("Failed to create descriptor sets");
			throw std::runtime_error("Failed to create descriptor sets");
		}

		this->setNumber = layout.setNumber;

	}

	template <class T>
	void AssociateUniformBuffers(VkDevice device, std::vector<UniformBuffer<T>> buffer, float offset, uint32_t binding)
	{

		std::vector<VkDescriptorBufferInfo> infos;

		for (int i = 0; i < buffer.size(); i++)
		{
			VkDescriptorBufferInfo VPbufferInfo = {};
			VPbufferInfo.buffer = buffer[i].buffer;
			VPbufferInfo.offset = offset;
			VPbufferInfo.range = sizeof(T);
			infos.push_back(VPbufferInfo);
		}


		VkWriteDescriptorSet mvpSetWrite = {};
		mvpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		mvpSetWrite.dstSet = m_descriptorSet;
		mvpSetWrite.dstBinding = binding;
		mvpSetWrite.dstArrayElement = 0;
		mvpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		mvpSetWrite.descriptorCount = static_cast<uint32_t>(buffer.size());
		mvpSetWrite.pBufferInfo = infos.data();


		std::vector< VkWriteDescriptorSet> setWrites = { mvpSetWrite };
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
	}

	template <class T>
	void AssociateDynamicUniformBuffers(VkDevice device, std::vector<DynamicUniformBuffer<T>>& buffer, float offset, uint32_t binding)
	{

		std::vector<VkDescriptorBufferInfo> infos;

		for (int i = 0; i < buffer.size(); i++)
		{
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = buffer[i].buffer;
			bufferInfo.offset = offset;
			bufferInfo.range = buffer[i].m_uniformAlignment;
			infos.push_back(bufferInfo);
		}


		VkWriteDescriptorSet mvpSetWrite = {};
		mvpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		mvpSetWrite.dstSet = m_descriptorSet;
		mvpSetWrite.dstBinding = binding;
		mvpSetWrite.dstArrayElement = 0;
		mvpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		mvpSetWrite.descriptorCount = static_cast<uint32_t>(buffer.size());
		mvpSetWrite.pBufferInfo = infos.data();


		std::vector< VkWriteDescriptorSet> setWrites = { mvpSetWrite };
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
	}

	void AssociateTextureSamplerCombo(VkDevice device, std::vector<Texture2D>& images, uint32_t binding, VkSampler sampler)
	{
		std::vector<VkImageView> views;
		for (Texture2D t : images)
			views.push_back(t.m_imageView);

		AssociateTextureSamplerCombo(device, views, binding, sampler);

	}

	void AssociateTextureSamplerCombo(VkDevice device, std::vector<VkImageView>& images, uint32_t binding, VkSampler sampler)
	{
		std::vector<VkDescriptorImageInfo> infos;
		for (int i = 0; i < images.size(); i++)
		{ 
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageView = images[i];
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.sampler = sampler;
			infos.push_back(imageInfo);
		}
		

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_descriptorSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = static_cast<uint32_t>(infos.size());
		descriptorWrite.pImageInfo = infos.data();


		//Image-sampler Descriptor
		std::vector< VkWriteDescriptorSet> setWrites = { descriptorWrite };
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
	}

	void AssociateTexture(VkDevice device, std::vector<VkImageView>& images, uint32_t binding)
	{
		std::vector<VkDescriptorImageInfo> infos;
		for (int i = 0; i < images.size(); i++)
		{
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageView = images[i];
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.sampler = nullptr;
			infos.push_back(imageInfo);
		}


		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_descriptorSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		descriptorWrite.descriptorCount = static_cast<uint32_t>(infos.size());
		descriptorWrite.pImageInfo = infos.data();


		//Image-sampler Descriptor
		std::vector< VkWriteDescriptorSet> setWrites = { descriptorWrite };
		vkUpdateDescriptorSets(device, setWrites.size(), setWrites.data(), 0, nullptr);
	}

	void Bind(VkCommandBuffer cmdBuffer, VkPipelineLayout layout)
	{
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			layout, setNumber,
			1,
			&m_descriptorSet, 0, nullptr);
	}

	VkDescriptorSet m_descriptorSet;
	uint32_t setNumber;

};