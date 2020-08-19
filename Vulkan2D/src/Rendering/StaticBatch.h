#pragma once

#include "UniformBuffer.h"
#include "Vertex.h"
#include <map>
#include <functional>
#include "VkContext.h"
#include "..\Core\Timer.h"

#define BATCH_MAX_VERTICES 200000

class MeshRenderer;
class GraphicsPipeline;
class Material;

class SingleBatch
{
public:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	UniformBuffer<Vertex> vertexBuffer;
	UniformBuffer<uint32_t> indexBuffer;
	uint32_t indexCount;

	uint32_t budget;

	uint32_t vIndex = 0;
	uint32_t iIndex = 0;

	SingleBatch()
	{
		budget = BATCH_MAX_VERTICES;
		vertices.reserve(BATCH_MAX_VERTICES);
		indices.reserve(BATCH_MAX_VERTICES * 2);

	}

	bool IsBudgetAvailable(size_t comingIn)
	{
		return comingIn < budget;
	}

	void Add(std::vector<Vertex> v, std::vector<uint32_t> in)
	{
		vertices.insert(vertices.end(), v.begin(), v.end());
		indices.insert(indices.end(), in.begin(), in.end());
		budget -= v.size();
		indexCount = indices.size();
	}

	void Destroy()
	{
		vertexBuffer.Destroy();
		indexBuffer.Destroy();

	}

	void PrepareBuffers()
	{
		vertices.shrink_to_fit();
		indices.shrink_to_fit();
		{
			UniformBuffer<Vertex> staging;
			staging.Create(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertices.size());		

			staging.Update(VkContext::Instance().GetLogicalDevice(), vertices.data());

			vertexBuffer.Create(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VMA_MEMORY_USAGE_GPU_ONLY, vertices.size());
		
			VkUtils::MemoryUtils::CopyBuffer(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(),
				VkContext::Instance().GetCommandLargerPool(), staging.buffer, vertexBuffer.buffer, staging.bufferSize);
			staging.Destroy();
		}

		{
			UniformBuffer<uint32_t> stage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, static_cast<uint32_t>(indices.size()));
			stage.Update(VkContext::Instance().GetLogicalDevice(), indices.data());
			if (indexBuffer.Create(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, indices.size()))
				Logger::LogError("Buffer in memory not mappable!");

			VkUtils::MemoryUtils::CopyBuffer(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(),
				VkContext::Instance().GetCommandLargerPool(), stage.buffer, indexBuffer.buffer, stage.bufferSize);
			stage.Destroy();
		}

		vertices.clear();
		indices.clear();
	}

};

class StaticBatch
{
public:
	StaticBatch() {};
	void PrepareStaticBuffers();
	void AddMeshRenderer(MeshRenderer* mr);
	void RenderBatches(int imageIndex);
	void RenderBatches(int imageIndex, GraphicsPipeline* pipeline);
	void DestroyBuffers();

	std::map<GraphicsPipeline*, std::map<Material*, std::vector<SingleBatch>>>			m_batches; 
};