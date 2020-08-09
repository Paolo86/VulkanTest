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
	Material(std::string name) : m_name(name) {}
	~Material();
	void Create(GraphicsPipeline* pipeline, std::vector<std::string> textureNames);
	void Destroy();
	void AddTextures(std::vector<std::string> fileNames);
	void Bind(VkCommandBuffer cmdBuffer);

	DescriptorSet m_samplerDescriptorSets;
	std::vector<Texture2D> m_textures;

	GraphicsPipeline* m_pipeline;
	void CreateSamplerDescriptorSet();

	std::string m_name;

};