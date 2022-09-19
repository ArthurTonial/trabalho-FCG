#pragma once
#include "BaseComponent.h"
#include "Scene.h"


class MeshRenderer : public BaseComponent
{
	Mesh* objectMesh;
	GLuint VAO = 0;
	Material* material;
	GLuint n_idx;

public:
	MeshRenderer(unsigned int oid, unsigned int cid, Transform* myobjTransform) : BaseComponent(oid, cid, myobjTransform)
	{

	}

	void initComponent();

	void execute();

	void endComponent();

	void setMesh(Mesh* m) { objectMesh = m; };
	void setMaterial(Material* s) { material = s; };
};

