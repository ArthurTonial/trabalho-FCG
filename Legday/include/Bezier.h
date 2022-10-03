#pragma once
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <algorithm>

using namespace glm;

class Bezier {
public:
	std::vector<vec3> control_points;

	Bezier() :
		control_points(std::vector<vec3>())
	{}
	Bezier(std::vector<vec3> cp) :
		control_points(cp)
	{}
	Bezier(vec3 p1, vec3 p2, vec3 p3, vec3 p4) :
		control_points(std::vector<vec3> {p1, p2, p3, p4})
	{}
	
	vec3 getPosition(float t) {
		vec3 c = control_points[0] * powf(1 - t, 3)
			   + control_points[1] * powf(1 - t, 2) * 3.0f * t
			   + control_points[2] * powf(1 - t, 1) * 3.0f * t * t
			   + control_points[3] * powf(t, 3);

		return c;
	}

	vec3 getTangent(float t) {
		return normalize(getPosition(t) - getPosition(t - 0.1));
	}
};