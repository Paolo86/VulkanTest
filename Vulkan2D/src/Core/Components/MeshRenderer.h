#pragma once
#include "..\..\Rendering\Mesh.h"
#include "..\..\Rendering\Material.h"
#include "..\..\Rendering\CommonStructs.h"
class MeshRenderer
{

public:
	MeshRenderer() {}
	MeshRenderer(Mesh* mesh, Material* material)
	{
		this->m_mesh = mesh;
		this-> m_material = material;
	}

	void SetMesh(Mesh* mesh)
	{
		this->m_mesh = mesh;

	}

	void SetMaterial(Material* mat)
	{
		this->m_material = mat;

	}

	UboModel uboModel;

	Mesh* m_mesh;
	Material* m_material;
};