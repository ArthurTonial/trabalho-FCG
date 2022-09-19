#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "BaseObject.h"
#include "BaseComponent.h"
#include "camera.h"

using namespace glm;

class Scene
{
public:

	static Camera* mainCamera;
	static vector<BaseObject*> objects;

	Transform getObjTransform(unsigned int oid) {
		return objects[oid]->getTransform();
	}
	
};

