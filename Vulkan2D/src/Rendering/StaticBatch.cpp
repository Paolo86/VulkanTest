#include "StaticBatch.h"
#include "GraphicsPipeline.h"
#include "Material.h"
#include "..\Core\Components\MeshRenderer.h"
#include "VkContext.h"
#include "..\Utils\Logger.h"
#include "..\Lighting\LightManager.h"
void StaticBatch::AddMeshRenderer(MeshRenderer* meshRenderer)
{
	std::vector<Vertex> vertices;
	for (int v = 0; v < meshRenderer->m_mesh->GetVertexCount(); v++)
	{
		Vertex vertex = meshRenderer->m_mesh->GetVertices()[v];
		vertex.pos = meshRenderer->uboModel.model * glm::vec4(vertex.pos, 1.0);
		vertex.normal = meshRenderer->uboModel.model * glm::vec4(vertex.normal, 0.0);
		vertex.tangent = meshRenderer->uboModel.model * glm::vec4(vertex.tangent, 0.0);
		vertex.binormal = meshRenderer->uboModel.model * glm::vec4(vertex.binormal, 0.0);
		vertices.push_back(vertex);
	}


	//Get an available batch
	if (m_batches[meshRenderer->m_material->GetPipelineUsed()][meshRenderer->m_material].size() == 0)
	{
		//No batches, create one
		SingleBatch batch;
		std::vector<uint32_t> indices;

		if (batch.IsBudgetAvailable(vertices.size()))
		{
			for (int i = 0; i < meshRenderer->m_mesh->GetIndexCount(); i++)
			{
				indices.push_back(meshRenderer->m_mesh->GetIndices()[i] + batch.vertices.size());
			}
			batch.Add(vertices, indices);
		}
		else
		{
			//This is a new batch. If the budget is small, this mesh is too big for this static batcher!
			throw std::runtime_error("Mesh too big!");
		}
		m_batches[meshRenderer->m_material->GetPipelineUsed()][meshRenderer->m_material].push_back(batch);
	}
	else
	{
		bool found = false;
		for (int i = 0; i < m_batches[meshRenderer->m_material->GetPipelineUsed()][meshRenderer->m_material].size(); i++)
		{
			if (m_batches[meshRenderer->m_material->GetPipelineUsed()][meshRenderer->m_material][i].IsBudgetAvailable(vertices.size()))
			{
				std::vector<uint32_t> indices;
				for (int j = 0; j < meshRenderer->m_mesh->GetIndexCount(); j++)
				{
					indices.push_back(meshRenderer->m_mesh->GetIndices()[j] + m_batches[meshRenderer->m_material->GetPipelineUsed()][meshRenderer->m_material][i].vertices.size());
				}

				m_batches[meshRenderer->m_material->GetPipelineUsed()][meshRenderer->m_material][i].Add(vertices, indices);
				found = true;
				break;
			}
		}
		if (!found)
		{
			//If it gets here, no batch was found with enough budget
			//No batches, create one
			//No batches, create one
			SingleBatch batch;
			std::vector<uint32_t> indices;

			if (batch.IsBudgetAvailable(vertices.size()))
			{
				for (int i = 0; i < meshRenderer->m_mesh->GetIndexCount(); i++)
				{
					indices.push_back(meshRenderer->m_mesh->GetIndices()[i] + batch.vertices.size());
				}
				batch.Add(vertices, indices);
			}
			else
			{
				//This is a new batch. If the budget is small, this mesh is too big for this static batcher!
				throw std::runtime_error("Mesh too big!");
			}
			m_batches[meshRenderer->m_material->GetPipelineUsed()][meshRenderer->m_material].push_back(batch);
		}

	}


}

void StaticBatch::DestroyBuffers()
{
	for (auto pipIt = m_batches.begin(); pipIt != m_batches.end(); pipIt++)
	{
		for (auto material = pipIt->second.begin(); material != pipIt->second.end(); material++)
		{
			for(int i=0; i< m_batches.size(); i++)
				m_batches[pipIt->first][material->first][i].Destroy();
		}
	}
}

void StaticBatch::RenderBatches(int imageIndex, GraphicsPipeline* pipeline)
{
	UboModel identity;
	identity.model = glm::mat4(1);

	vkCmdPushConstants(VkContext::Instance().GetCommandBuferAt(imageIndex), pipeline->GetPipelineLayout(),
		VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UboModel), &identity);
	
	for (auto material = m_batches[pipeline].begin(); material != m_batches[pipeline].end(); material++)
	{
		material->first->Bind(VkContext::Instance().GetCommandBuferAt(imageIndex));

		//For each batch using this pipeline and material....
		for (int i = 0; i < m_batches[pipeline][material->first].size(); i++)
		{

			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(VkContext::Instance().GetCommandBuferAt(imageIndex), 0, 1, &material->second[i].vertexBuffer.buffer, &offset);
			vkCmdBindIndexBuffer(VkContext::Instance().GetCommandBuferAt(imageIndex), material->second[i].indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(VkContext::Instance().GetCommandBuferAt(imageIndex),	material->second[i].indexCount, 1, 0, 0, 0);

		}
	}
}


void StaticBatch::RenderBatches(int imageIndex)
{
	UboModel identity;
	identity.model = glm::mat4(1);
	//Logger::LogInfo("Start render batch");
	for (auto pipeline = m_batches.begin(); pipeline != m_batches.end(); pipeline++)
	{
		vkCmdPushConstants(VkContext::Instance().GetCommandBuferAt(imageIndex), pipeline->first->GetPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UboModel), &identity);

		//pipeline->first->Bind(VkContext::Instance().GetCommandBuferAt(imageIndex), imageIndex);

		for (auto material = pipeline->second.begin(); material != pipeline->second.end(); material++)
		{
			material->first->Bind(VkContext::Instance().GetCommandBuferAt(imageIndex));

			//For each batch using this pipeline and material....
			for (int i = 0; i < m_batches[pipeline->first][material->first].size(); i++)
			{
				VkDeviceSize offset = 0;
				vkCmdBindVertexBuffers(VkContext::Instance().GetCommandBuferAt(imageIndex), 0, 1, &material->second[i].vertexBuffer.buffer, &offset);
				vkCmdBindIndexBuffer(VkContext::Instance().GetCommandBuferAt(imageIndex), material->second[i].indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(VkContext::Instance().GetCommandBuferAt(imageIndex),
					material->second[i].indexCount, 1, 0, 0, 0);

			}
		}
	}
}


void StaticBatch::PrepareStaticBuffers()
{
	for (auto pipeline = m_batches.begin(); pipeline != m_batches.end(); pipeline++)
	{

		for (auto material = pipeline->second.begin(); material != pipeline->second.end(); material++)
		{
			Logger::LogInfo("Total batches: ", m_batches[pipeline->first][material->first].size());

			for (int i = 0; i < m_batches[pipeline->first][material->first].size(); i++)
			{
				//Logger::LogInfo("Preparing one batch");
				//Logger::LogInfo("\tVertices ", m_batches[pipeline->first][material->first][i].vertices.size());
				//Logger::LogInfo("\tIndices ", m_batches[pipeline->first][material->first][i].indices.size());
				m_batches[pipeline->first][material->first][i].PrepareBuffers();
			}
		}
	}
}

