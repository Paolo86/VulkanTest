#pragma once
#include <vulkan/vulkan.hpp>
#include "VkUtils.h"
#include <vk_mem_alloc.h>
#include "..\Utils\Logger.h"
template <class T>
class UniformBuffer
{
public:
	UniformBuffer(VkBufferUsageFlags usageFlag, VmaMemoryUsage memProps, int count = 1);
	UniformBuffer() {}

	bool Create(VkBufferUsageFlags usageFlag, VmaMemoryUsage memProps, int count = 1)
	{
		bufferSize = sizeof(T) * count;

		return VkUtils::MemoryUtils::CreateBufferVMA(bufferSize, usageFlag, memProps, &buffer, &bufferMemoryVMA);
	}

	bool Create(VkBufferUsageFlags usageFlag, VkMemoryPropertyFlags preferred, VkMemoryPropertyFlags required, int count = 1)
	{
		bufferSize = sizeof(T) * count;

		return VkUtils::MemoryUtils::CreateBufferVMA(bufferSize, usageFlag, preferred, required, &buffer, &bufferMemoryVMA);
	}
	void Update(VkDevice device, T* newData)
	{
		void* data;
		vmaMapMemory(VkUtils::MemoryUtils::allocator, bufferMemoryVMA, &data);
		memcpy(data, newData, (size_t)bufferSize);
		vmaUnmapMemory(VkUtils::MemoryUtils::allocator, bufferMemoryVMA);

	}

	void Destroy()
	{
		VkUtils::MemoryUtils::DestroyBuffer(buffer, bufferMemoryVMA);
	}


	VkDeviceSize bufferSize;
	VkBuffer buffer;
	VmaAllocation bufferMemoryVMA;
};


template <class T>

UniformBuffer<T>::UniformBuffer(VkBufferUsageFlags usageFlag, VmaMemoryUsage memProps, int count)
{
	bufferSize = sizeof(T) * count;

	VkUtils::MemoryUtils::CreateBufferVMA(bufferSize, usageFlag, memProps, &buffer, &bufferMemoryVMA);
}




