#pragma once

#include <iostream>
#include "Mesh.h"
#include "Transform.h"

class BaseComponent
{

	unsigned int cid;
	unsigned int oid;
	

public:
	Transform* objectTransform;
	Transform componentTransform;

	BaseComponent(unsigned int oid, unsigned int cid, Transform* myobjTransform) : 
		oid(oid), 
		cid(cid), 
		objectTransform(myobjTransform) 
	{
		// put it into MainProgram.activeScene.objects[oid].push(this)
	}

	virtual void initComponent() {
		std::cout << "no definition for 'initComponent' in derived component" << oid << "\n";
	}

	virtual void execute() {
		std::cout << "no definition for 'execute' in derived component" << oid << "\n";
	}

	virtual void endComponent() {
		std::cout << "no definition for 'endComponent' in derived component" << oid << "\n";
	}

	unsigned int getcid() { return cid; }
	unsigned int getoid() { return oid; }
	Transform getTransform() { return componentTransform; }
	void setoid(unsigned int id) { oid = id; }
	void setTransform(Transform t) { 
		componentTransform = t; 
	}

	Transform getTrueTransform() {
		return Transform(
			objectTransform->position + (objectTransform->rotation * (objectTransform->scale * componentTransform.position)),
			objectTransform->rotation * componentTransform.rotation,
			componentTransform.scale * objectTransform->scale);
	}
};

