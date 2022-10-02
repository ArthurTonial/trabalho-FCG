#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "BaseObject.h"
#include "BaseComponent.h"
#include "camera.h"
#include "Renderer.h"

using namespace glm;

struct Ray {
	vec3 direction;
	vec3 start;
};

class Collision
{
public:

	static bool isColliding(vec3 bbox_min_a, vec3 bbox_max_a, vec3 bbox_min_b, vec3 bbox_max_b, Transform tr_a, Transform tr_b) {

	}

	static bool isColliding(vec3 bbox_min_a, vec3 bbox_max_a, Transform tr_a, Ray r) {

	}

	static RenderObject* testCollisions(Ray r) {
		int n = Renderer::renderQ.size();

		while (n--) {
			RenderObject* cur = Renderer::renderQ.front();
			Renderer::renderQ.pop();
			Renderer::renderQ.push(cur);


		}
	}

};