#pragma once
#include "../Rendering/Texture2D.h"
#include "stb_image.h"
#include "../Rendering/GraphicsPipeline.h"
#include "..\Rendering\Mesh.h"
#include <map>

struct MeshData
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	MeshData() {}
	MeshData(std::vector<Vertex> v, std::vector<uint32_t> i) : vertices(v), indices(i) {}
};

class ResourceManager
{
public:
	static Texture2D CreateSampleTextureFromFile(std::string fileName, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
	static Texture2D CreateDepthBufferImage();
	static stbi_uc* LoadTexture(std::string fileName, int* width, int* height, int* channels);

	static MeshData LoadModel(std::string filePath, std::string meshName);

	static void CreatePipelines();
	static void CreateMeshes();

	static void DestroyAll();

	static GraphicsPipeline* GetPipeline(std::string name) { return allPipelines[name].get(); }
	static Mesh* GetMesh(std::string name) { return allMeshes[name].get(); }

private:
	static std::map<std::string, std::unique_ptr<GraphicsPipeline>> allPipelines;
	static std::map<std::string, std::unique_ptr<Mesh>> allMeshes;


};