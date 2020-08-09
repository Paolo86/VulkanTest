#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "CommonStructs.h"

#include "GraphicsPipeline.h"
#include "PushConstant.h"
#include "DescriptorSet.h"
#include "TextureSampler.h"
#include "Texture2D.h"


class Material
{
public:
	friend class Vk;
	Material(std::string name) : m_name(name) {}
	~Material();
	void Create(GraphicsPipeline* pipeline, std::vector<std::string> textureNames);
	void Destroy();
	void AddTextures(std::vector<std::string> fileNames);
	void Bind(VkCommandBuffer cmdBuffer);


	void SetTint(float r, float g, float b, float a = 1.0);


	std::string m_name;
private:
	void CreateSamplerDescriptorSet();
	GraphicsPipeline* m_pipeline;
	UniformBuffer<MaterialProps> propertiesBuffer;
	MaterialProps materialProperties;
	DescriptorSet m_samplerDescriptorSets;
	std::vector<Texture2D> m_textures;
};