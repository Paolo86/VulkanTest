#pragma once
#include "../Rendering/Texture2D.h"
#include "stb_image.h"
#include "../Rendering/GraphicsPipeline.h"
#include "..\Rendering\Mesh.h"
#include <map>

class ResourceManager
{
public:
	static Texture2D CreateSampleTextureFromFile(std::string fileName, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
	static Texture2D CreateDepthBufferImage();
	static stbi_uc* LoadTexture(std::string fileName, int* width, int* height, int* channels);

	static void CreatePipelines();
	static void CreateMeshes();

	static GraphicsPipeline* GetPipeline(std::string name) { return allPipelines[name].get(); }

private:
	static std::map<std::string, std::unique_ptr<GraphicsPipeline>> allPipelines;
	static std::map<std::string, std::unique_ptr<Mesh>> allMeshes;


};