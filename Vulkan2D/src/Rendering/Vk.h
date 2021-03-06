#pragma once

#include "VkContext.h"
#include "..\Core\Components\MeshRenderer.h"
#include "stb_image.h"
#include "UniformBuffer.h"
#include "TextureSampler.h"
#include "VkDebugMessanger.h"
#include "FrameBuffer.h"
#include "StaticBatch.h"
#include <map>
#include <set>

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
		std::vector<DynamicUniformBuffer<_ViewProjection>> m_dynamicBuffer;;
		TextureSampler m_textureSampler;
		VkDescriptorPool m_descriptorPool;
		VkDescriptorPool m_samplerDescriptorPool;

		void AddMeshRenderer(MeshRenderer* meshRenderer, bool isStatic);
		void UpdateView(glm::vec3 pos, glm::vec3 dir);

private:
		void CreateDescriptorPool();

		std::map<GraphicsPipeline*, std::map<Mesh*, std::map<Material*,std::set<MeshRenderer*>>>> m_dynamicObjrenderMap;
		std::set<GraphicsPipeline*> m_allPipelineUsed;
		
		StaticBatch m_staticBatch;

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
		void CreateFramebuffers();
		void RenderCmds(uint32_t currentImage);
		void CreateUniformBuffers();


};