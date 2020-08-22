#pragma once
#include "..\..\Rendering\Mesh.h"
#include "..\..\Rendering\Material.h"
#include "..\..\Rendering\CommonStructs.h"

class InstanceRenderer
{

public:
	InstanceRenderer();
	InstanceRenderer(Mesh* mesh, Material* material, std::vector<InstanceTransform> transforms);
	void Create(std::vector<InstanceTransform> transforms);
	void SetMesh(Mesh* mesh);
	void Destroy() {} //TODO: implement

	void SetMaterial(Material* mat);
	void BindBuffers(VkCommandBuffer cmdBuffer);
	void Draw(int imageIndex);
	Mesh* m_mesh;
	Material* m_material;
	UniformBuffer<InstanceTransform> m_instanceTransformsBuffer;
	std::vector<InstanceTransform> m_instanceTransforms;

};