#include "Material.h"
#include "..\Utils\FileUtils.h"
#include "VkUtils.h"
#include "Vk.h"
#include "../Asset/ResourceManager.h"



void Material::Create(GraphicsPipeline* pipeline, std::vector<std::string> textureNames) 
{
	m_pipeline = pipeline;
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



void Material::CreateSamplerDescriptorSet()
{

	//Texture sampler descriptor set layout
	
	m_samplerDescriptorSets.CreateDescriptorSet(VkContext::Instance().GetLogicalDevice(), { m_pipeline->GetSamplerLayout() }, Vk::Instance().m_samplerDescriptorPool);

	m_samplerDescriptorSets.AssociateTextureSamplerCombo(VkContext::Instance().GetLogicalDevice(), m_textures, 0, Vk::Instance().m_textureSampler.m_sampler);

}

