#pragma once
#include "BaseComponent.h"
#include "Scene.h"


class MeshRenderer : public BaseComponent
{
	Material*		objectMaterial;
	Mesh*			objectMesh;
	RenderObject	renderObj;

public:
	MeshRenderer(unsigned int oid, unsigned int cid, Transform* myobjTransform) : BaseComponent(oid, cid, myobjTransform)
	{
		
	}

	void initComponent();

	void execute();

	void endComponent();

	void setMesh(Mesh* m) { objectMesh = m; };
	void setMaterial(Material* s) { objectMaterial = s; };
};

