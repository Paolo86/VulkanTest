#pragma once

#include "VkContext.h"
#include "Mesh.h"
#include "stb_image.h"
#include "UniformBuffer.h"
#include "TextureSampler.h"
#include "VkDebugMessanger.h"
#include "FrameBuffer.h"

class Vk
{
	public:
		static Vk& Instance();
		Vk();
		~Vk();
		void Init();
		void Destroy();
		void Draw();

		//Change image layout

		VkRenderPass m_renderPass;
		std::vector<UniformBuffer<_ViewProjection>> m_VPUniformBuffers;
		TextureSampler m_textureSampler;
		VkDescriptorPool m_descriptorPool;
		VkDescriptorPool m_samplerDescriptorPool;

private:
		void CreateDescriptorPool();

		Mesh firstMesh;
		Mesh secondMesh;
		std::vector<Mesh> m_meshes;
		static std::unique_ptr<Vk> m_instance;

		UboModel* m_modelTransferSpace;
		/*Params*/	

		std::vector<FrameBuffer> m_swapChainFramebuffers;

		Texture2D m_depthBufferImage;

		_ViewProjection ViewProjection;

		std::vector<VkBuffer> m_modelDynamicPuniformBuffer;
		std::vector<VkDeviceMemory> m_modelDynamicuniformBufferMemory;

		/*Methods*/
		void CreateRenderPass();
		void CreateDepthBufferImage();
		void CreateFramebuffers();
		void RenderCmds(uint32_t currentImage);
		void CreateUniformBuffers();
		void AllocateDynamicBufferTransferSpace();

		size_t m_modelUniformAlignment;

};