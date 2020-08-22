#pragma once
#include "Mesh.h"
#include "Material.h"
#include "CommonStructs.h"


struct ModelMatrices
{
	glm::mat4 matrices[2000];
};

class UBOBatch
{

public:
	UBOBatch();
	UBOBatch(Mesh* mesh, Material* material);
	void Create(std::vector<InstanceTransform>& transforms);
	void SetMesh(Mesh* mesh);
	void Destroy();

	void SetMaterial(Material* mat);
	void BindBuffers(VkCommandBuffer cmdBuffer);
	void Draw(int imageIndex);
	Mesh* m_mesh;
	Material* m_material;
	UniformBuffer<ModelMatrices> m_instanceTransformsBuffer;
	ModelMatrices m_instanceTransforms;
	DescriptorSet m_descriptorSet;
	int instanceCount;

};