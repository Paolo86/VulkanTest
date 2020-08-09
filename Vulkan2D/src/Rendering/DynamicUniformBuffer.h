#pragma once
#include <vulkan/vulkan.hpp>
#include "VkUtils.h"
#include <vk_mem_alloc.h>

template <class T>
class DynamicUniformBuffer
{
public:
	DynamicUniformBuffer(VkBufferUsageFlags usageFlag, VmaMemoryUsage memProps, int count = 1);
	DynamicUniformBuffer() {}

	void Create(VkBufferUsageFlags usageFlag, VmaMemoryUsage memProps, int count = 1)
	{

		m_uniformAlignment = (sizeof(T) + VkContext::Instance().GetMinUniformBufferOffset() - 1) & ~(VkContext::Instance().GetMinUniformBufferOffset() - 1);
		bufferSize = m_uniformAlignment * count;
		//Fixed space ot hold all model matrices of all objects
		m_modelTransferSpace = (T*)_aligned_malloc(m_uniformAlignment * count, m_uniformAlignment);

		VkUtils::MemoryUtils::CreateBufferVMA(bufferSize, usageFlag, memProps, &buffer, &bufferMemoryVMA);
	}
	void Update(VkDevice device, std::vector<T> dataIn)
	{
		for (size_t i = 0; i < dataIn.size(); i++)
		{
			T* thisMem = (T*)((uint64_t), m_modelTransferSpace + (i * m_uniformAlignment));
			*thisMem = dataIn[i];
		}

		void* data;
		vmaMapMemory(VkUtils::MemoryUtils::allocator, bufferMemoryVMA, &data);
		memcpy(data, &m_modelTransferSpace, dataIn.size() * m_uniformAlignment);
		vmaUnmapMemory(VkUtils::MemoryUtils::allocator, bufferMemoryVMA);
	}

	void Destroy()
	{
		VkUtils::MemoryUtils::DestroyBuffer(buffer, bufferMemoryVMA);
		_aligned_free(m_modelTransferSpace);

	}


	VkDeviceSize bufferSize;
	VkBuffer buffer;
	VmaAllocation bufferMemoryVMA;
	size_t m_uniformAlignment;
	T* m_modelTransferSpace;

};


template <class T>

DynamicUniformBuffer<T>::DynamicUniformBuffer(VkBufferUsageFlags usageFlag, VmaMemoryUsage memProps, int count)
{
	bufferSize = sizeof(T) * count;

	VkUtils::MemoryUtils::CreateBufferVMA(bufferSize, usageFlag, memProps, &buffer, &bufferMemoryVMA);
}




