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
	SetUVScale(1,1);
	if(textureNames.size() > 0)
	AddTextures(textureNames);
}

void Material::AddTextures(std::vector<std::string> fileNames)
{
	for (std::string name : fileNames)
	{

		Texture2D t = ResourceManager::CreateSampleTextureFromFile(name);
		m_textures.push_back(t);
	}
	if(fileNames.size() > 0)
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
	materialProperties.uvScale.r = r;
	materialProperties.uvScale.g = g;
	propertiesBuffer.Update(VkContext::Instance().GetLogicalDevice(), &materialProperties);
}

void Material::SetPBRProps(float metallic, float rooughness, float ao)
{
	materialProperties.pbrProps.x = metallic;
	materialProperties.pbrProps.y = rooughness;
	materialProperties.pbrProps.z = ao;
	propertiesBuffer.Update(VkContext::Instance().GetLogicalDevice(), &materialProperties);
}



/*Material descriptor set for textures is index 1*/
void Material::Bind(VkCommandBuffer cmdBuffer)
{
	//Logger::LogInfo("\tBind material ",m_name);
	if(m_textures.size() > 0)
	m_samplerDescriptorSets.Bind(cmdBuffer, m_pipeline->GetPipelineLayout());
}


void Material::CreateMaterialPropsDescriptorSet()
{

}


void Material::CreateSamplerDescriptorSet()
{
	// Texture set
	VkDescriptorSetLayoutBinding imagesLayoutBinding = VkUtils::PipelineUtils::GetDescriptorLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		, 5, VK_SHADER_STAGE_FRAGMENT_BIT);

	VkDescriptorSetLayoutBinding materialPropertiesBinding = VkUtils::PipelineUtils::GetDescriptorLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
		, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

	DescriptorSetLayout layout;
	layout.setNumber = 2;
	layout.AddBinding({imagesLayoutBinding, materialPropertiesBinding}).Create(VkContext::Instance().GetLogicalDevice());
	
	m_samplerDescriptorSets.CreateDescriptorSet(VkContext::Instance().GetLogicalDevice(), layout, Vk::Instance().m_samplerDescriptorPool);
	m_samplerDescriptorSets.AssociateTextureSamplerCombo(VkContext::Instance().GetLogicalDevice(), m_textures, 0, Vk::Instance().m_textureSampler.m_sampler);

	m_samplerDescriptorSets.AssociateUniformBuffers<MaterialProps>(VkContext::Instance().GetLogicalDevice(), { propertiesBuffer }, 0, 1);
	layout.Destroy(VkContext::Instance().GetLogicalDevice());

}

