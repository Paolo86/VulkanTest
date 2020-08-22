#include "UBOBatch.h"
#include "VkContext.h"
#include "Vk.h"

UBOBatch::UBOBatch() {}

UBOBatch::UBOBatch(Mesh* mesh, Material* material)
{
	this->m_mesh = mesh;
	this->m_material = material;
}

void UBOBatch::BindBuffers(VkCommandBuffer cmdBuffer)
{
	VkDeviceSize offsets[] = { 0 };
	m_mesh->BindBuffers(cmdBuffer);
	m_descriptorSet.Bind(cmdBuffer, m_material->GetPipelineUsed()->GetPipelineLayout());

}

void UBOBatch::Create(std::vector<InstanceTransform>& transforms)
{
	instanceCount = transforms.size();

	for (int i = 0; i < transforms.size(); i++)
	{
		glm::mat4 translate;
		translate = glm::translate(translate, transforms[i].position);
		m_instanceTransforms.matrices[i] = translate;
	}

	UniformBuffer<ModelMatrices> staging(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY, 1);
	staging.Update(VkContext::Instance().GetLogicalDevice(), &m_instanceTransforms);
	m_instanceTransformsBuffer.Create(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 1);
	VkUtils::MemoryUtils::CopyBuffer(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(),
		VkContext::Instance().GetCommandPool(), staging.buffer, m_instanceTransformsBuffer.buffer, staging.bufferSize);
	staging.Destroy();


	//Layout
	VkDescriptorSetLayoutBinding modelMatricesBinding = VkUtils::PipelineUtils::GetDescriptorLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
		, 1, VK_SHADER_STAGE_VERTEX_BIT);

	DescriptorSetLayout layout;
	layout.setNumber = 3;
	layout.AddBinding({ modelMatricesBinding }).Create(VkContext::Instance().GetLogicalDevice());

	m_descriptorSet.CreateDescriptorSet(VkContext::Instance().GetLogicalDevice(), layout, Vk::Instance().m_descriptorPool);
	m_descriptorSet.AssociateUniformBuffers<ModelMatrices>(VkContext::Instance().GetLogicalDevice(), { m_instanceTransformsBuffer }, 0, 0);
}


void UBOBatch::SetMesh(Mesh* mesh)
{
	this->m_mesh = mesh;

}

void UBOBatch::Destroy()
{
	m_instanceTransformsBuffer.Destroy();
}

void UBOBatch::SetMaterial(Material* mat)
{
	this->m_material = mat;

}

void UBOBatch::Draw(int imageIndex)
{
	vkCmdDrawIndexed(VkContext::Instance().GetCommandBuferAt(imageIndex), m_mesh->GetIndexCount(), instanceCount, 0, 0, 0);

}
