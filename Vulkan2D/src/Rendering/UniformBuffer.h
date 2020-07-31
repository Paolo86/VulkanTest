#pragma once
#include <vulkan/vulkan.hpp>

template <class T>
class UniformBuffer
{
public:
	UniformBuffer(VkBufferUsageFlagBits usageFlag, VkMemoryPropertyFlags memProps);
	UniformBuffer() {}

	void Update(VkDevice device, T* data)
	{
		void* mapper;
		vkMapMemory(device, bufferMemory, 0, sizeof(T), 0, &mapper);
		memcpy(mapper, data, sizeof(T));
		vkUnmapMemory(device, bufferMemory);
	}

	void Destroy(VkDevice device)
	{
		vkDestroyBuffer(device, buffer, nullptr);
		vkFreeMemory(device, bufferMemory, nullptr);
	}


	VkDeviceSize bufferSize;
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
};

template <class T>
UniformBuffer<T>::UniformBuffer(VkBufferUsageFlagBits usageFlag, VkMemoryPropertyFlags memProps)
{
	bufferSize = sizeof(T);
	Vk::Instance().CreateBuffer(bufferSize, usageFlag, memProps, &buffer, &bufferMemory);
}


