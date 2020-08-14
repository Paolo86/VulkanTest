#include "Material.h"
#include "..\Utils\FileUtils.h"
#include "VkUtils.h"
#include "Vk.h"
#include "../Asset/ResourceManager.h"



void Material::Create(GraphicsPipeline* pipeline, std::vector<std::string> textureNames) 
{
	m_pipeline = pipeline;

	propertiesBuffer.Create(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, 1);
	SetTint(1, 1, 1);
	SetUVScale(2,2);
	AddTextures(textureNames);
}

void Material::AddTextures(std::vector<std::string> fileNames)
{
	for (std::string name : fileNames)
	{

		Texture2D t = ResourceManager::CreateSampleTextureFromFile(name);
		m_textures.push_back(t);
	}

	CreateSamplerDescriptorSet();
}


void Material::Destroy()
{
	for (size_t i = 0; i < m_textures.size(); i++)
	{
		m_textures[i].Destroy();
	}


}


Material::~Material()
{

}

void Material::SetTint(float r, float g, float b, float a)
{
	materialProperties.tint = glm::vec4(r,g,b,a);
	propertiesBuffer.Update(VkContext::Instance().GetLogicalDevice(), &materialProperties);

}

void Material::SetUVScale(float r, float g)
{
	materialProperties.uvScale = glm::vec2(r, g);
	propertiesBuffer.Update(VkContext::Instance().GetLogicalDevice(), &materialProperties);
}



/*Material descriptor set for textures is index 1*/
void Material::Bind(VkCommandBuffer cmdBuffer)
{
	//Logger::LogInfo("\tBind material ",m_name);


	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipeline->m_pipelineLayout, 1,
		1,
		&m_samplerDescriptorSets.m_descriptorSet, 0, nullptr);
}


void Material::CreateSamplerDescriptorSet()
{
	
	m_samplerDescriptorSets.CreateDescriptorSet(VkContext::Instance().GetLogicalDevice(), { m_pipeline->GetSamplerLayout() }, Vk::Instance().m_samplerDescriptorPool);
	m_samplerDescriptorSets.AssociateTextureSamplerCombo(VkContext::Instance().GetLogicalDevice(), m_textures, 0, Vk::Instance().m_textureSampler.m_sampler);


	m_samplerDescriptorSets.AssociateUniformBuffers<MaterialProps>(VkContext::Instance().GetLogicalDevice(), { propertiesBuffer }, 0, 1);

}

