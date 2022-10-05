#pragma once
#include "BaseObject.h"
#include "MeshRenderer.h"
#include "Collision.h"

class CubeObject : public BaseObject
{

public:
	bool debugMode;
	Mesh* objectMesh;
	Shader objectShader;
	MeshRenderer* ms;
	CollisionObject objectCollision;

	CubeObject(unsigned int oid) : BaseObject(oid)
	{
		objectMesh = new Mesh("meshes/cow.obj");
		objectShader = Shader("shaders/pbr.vs", "shaders/pbr.fs");
		objectCollision = CollisionObject(objectMesh, transform);

		ms = new MeshRenderer(oid, 0, &transform);
		ms->setMesh(objectMesh);
		ms->setMaterial(new Material(1.0f, 2.0f, 0.0f, vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), &objectShader));

		addComponent(ms);


		//this->addComponent(new MeshRenderer(oid, 0, &transform));
		//((MeshRenderer*)getComponents()[0])->setMesh(&cubeMesh);
		//((MeshRenderer*)getComponents()[0])->setMaterial(new Material(1.0f, 1.0f, 0.0f, vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	}

	void run() {
		if(debugMode)objectCollision.printHierarchy();
		return;
	}

};

