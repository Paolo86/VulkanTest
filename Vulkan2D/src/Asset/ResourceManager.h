#pragma once
#include "../Rendering/Texture2D.h"
#include "stb_image.h"

class ResourceManager
{
public:
	static Texture2D CreateSampleTextureFromFile(std::string fileName, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
	static Texture2D CreateDepthBufferImage();
	static stbi_uc* LoadTexture(std::string fileName, int* width, int* height, int* channels);



private:

};