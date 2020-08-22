#include "ResourceManager.h"
#include "..\Rendering\VkUtils.h"
#include "..\Rendering\VkContext.h"
#include "Pipelines\BasicPipeline.h"
#include "Pipelines\PBRPipeline.h"
#include "Pipelines\PBR_Instanced.h"
#include "Pipelines\Phong.h"
#include "TinyOBJLoader.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.uvs) << 1);
		}
	};
}


std::map<std::string, std::unique_ptr<GraphicsPipeline>> ResourceManager::allPipelines;
std::map<std::string, std::unique_ptr<Mesh>> ResourceManager::allMeshes;

stbi_uc* ResourceManager::LoadTexture(std::string fileName, int* width, int* height, int* channels)
{
	std::string fileLoc = "Textures/" + fileName;
	stbi_uc* image = stbi_load(fileLoc.c_str(), width, height, channels, STBI_rgb_alpha);

	if (!image)
		throw std::runtime_error("Failed to load texture " + fileName);

	return image;
}

Texture2D ResourceManager::CreateSampleTextureFromFile(std::string fileName, VkFormat format)
{
	int w, h, c;
	stbi_uc* imageData = LoadTexture(fileName, &w, &h, &c);

	Texture2D texture;
	VkDeviceSize size = w * h * 4; //Alpha channel is basically forced, so RGBA, 4 channels in total
	VkBuffer staging;
	VkDeviceMemory stagingMemory;
	VkUtils::MemoryUtils::CreateBuffer(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetPhysicalDevice(),
		size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging, &stagingMemory);

	void* data;
	vkMapMemory(VkContext::Instance().GetLogicalDevice(), stagingMemory, 0, size, 0, &data);
	memcpy(data, imageData, static_cast<size_t>(size));
	vkUnmapMemory(VkContext::Instance().GetLogicalDevice(), stagingMemory);

	texture.m_image = VkUtils::ImageUtils::CreateImage(VkContext::Instance().GetPhysicalDevice(), VkContext::Instance().GetLogicalDevice(), w, h,
		format, VK_IMAGE_TILING_OPTIMAL, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture.m_imageMemory);
	//Before copy, transition layout to optimal transfer
	VkUtils::ImageUtils::TransitionImageLayout(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(), VkContext::Instance().GetCommandPool(),
		texture.m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkUtils::ImageUtils::CopyImageBuffer(VkContext::Instance().GetLogicalDevice(), 
		VkContext::Instance().GetGraphicsTransferQ(), VkContext::Instance().GetCommandPool(), staging, texture.m_image, w, h);

	//After copy, transfer layout to shader readable
	VkUtils::ImageUtils::TransitionImageLayout(VkContext::Instance().GetLogicalDevice(), VkContext::Instance().GetGraphicsTransferQ(), 
		VkContext::Instance().GetCommandPool(), texture.m_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(VkContext::Instance().GetLogicalDevice(), staging, nullptr);
	vkFreeMemory(VkContext::Instance().GetLogicalDevice(), stagingMemory, nullptr);

	texture.m_imageView = VkUtils::ImageUtils::CreateImageView(VkContext::Instance().GetLogicalDevice(), texture.m_image, format, VK_IMAGE_ASPECT_COLOR_BIT);
	return texture;
}

Texture2D ResourceManager::CreateDepthBufferImage()
{
	Texture2D texture;
	VkFormat depthFormat = VkContext::Instance().ChooseSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	texture.m_image = VkUtils::ImageUtils::CreateImage(VkContext::Instance().GetPhysicalDevice(), VkContext::Instance().GetLogicalDevice(),
		VkContext::Instance().GetSwapChainExtent().width, VkContext::Instance().GetSwapChainExtent().height, depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture.m_imageMemory);

	texture.m_imageView = VkUtils::ImageUtils::CreateImageView(VkContext::Instance().GetLogicalDevice(), texture.m_image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	return texture;

}

void ResourceManager::CreatePipelines()
{
	allPipelines["Basic"] = std::unique_ptr<BasicPipeline>(new BasicPipeline());
	allPipelines["PBR"] = std::unique_ptr<PBRPipeline>(new PBRPipeline());
	allPipelines["PBRInstanced"] = std::unique_ptr<PBRInstancedPipeline>(new PBRInstancedPipeline());
	allPipelines["Phong"] = std::unique_ptr<PhongPipeline>(new PhongPipeline());
}

void ResourceManager::CreateMeshes()
{
	Vertex v1;
	v1.pos = glm::vec3(-0.5, -0.5, 0.0);
	v1.color = glm::vec3(1.0, 0.0, 0.0);
	v1.uvs = glm::vec2(0, 0);
	v1.normal = glm::vec3(0, 0, 1);

	Vertex v2;
	v2.pos = glm::vec3(0.5, -0.5, 0.0);
	v2.color = glm::vec3(0.0, 1.0, 0.0);
	v2.uvs = glm::vec2(1, 0);
	v2.normal = glm::vec3(0, 0, 1);

	Vertex v3;
	v3.pos = glm::vec3(0.5, 0.5, 0.0);
	v3.color = glm::vec3(0.0, 0.0, 1.0);
	v3.uvs = glm::vec2(1, 1);
	v3.normal = glm::vec3(0, 0, 1);

	Vertex v4;
	v4.pos = glm::vec3(-0.5, 0.5, 0.0);
	v4.color = glm::vec3(0.0, 0.0, 1.0);
	v4.uvs = glm::vec2(0, 1);
	v4.normal = glm::vec3(0, 0, 1);

	std::vector<Vertex> vertices1 = { v1,v2,v3, v4 };
	std::vector<uint32_t> indices1 = { 0,1,2,2,3,0 };

	std::vector<Vertex> vertices2 = { v1,v3,v4 };
	std::vector<uint32_t> indices2 = { 0,1,2 };

	allMeshes["Quad"] = std::unique_ptr<Mesh>(new Mesh(vertices1, indices1));
}

void ResourceManager::DestroyAll()
{
	for (auto it = allMeshes.begin(); it != allMeshes.end(); it++)
		it->second->DestroyVertexBuffer();

	for (auto it = allPipelines.begin(); it != allPipelines.end(); it++)
		it->second->Destroy(VkContext::Instance().GetLogicalDevice());
}




MeshData ResourceManager::LoadModel(std::string path, std::string meshName)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
		throw std::runtime_error(warn + err);
	}
	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	MeshData data;
	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			vertex.pos = {
			attrib.vertices[3 * index.vertex_index + 0],
			attrib.vertices[3 * index.vertex_index + 1],
			attrib.vertices[3 * index.vertex_index + 2]
					};

			vertex.uvs = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.normal = {
			attrib.vertices[3 * index.normal_index + 0],
			attrib.vertices[3 * index.normal_index + 1],
			attrib.vertices[3 * index.normal_index + 2]
			};
			

			vertex.color = { 1.0f, 1.0f, 1.0f };
			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(data.vertices.size());
				data.vertices.push_back(vertex);
			}

			data.indices.push_back(uniqueVertices[vertex]);
		}
	}

	allMeshes[meshName] = std::unique_ptr<Mesh>(new Mesh(data.vertices, data.indices));
	return data;

}


