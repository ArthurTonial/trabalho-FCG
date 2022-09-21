#pragma once
#include "BaseComponent.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace glm;

class PlayerController : public BaseComponent
{
	enum KEYS
	{
		SHIFT,
		SPACE,
		W,
		A,
		S,
		D
	};

	float deltaTime;
	float lastTime;

public:
	vec3 acceleration;
	vec3 velocity;
	float damp;
	float speed;
	float acc;
	bool* is_pressed;

	PlayerController(unsigned int oid, unsigned int cid, Transform* myobjTransform) : BaseComponent(oid, cid, myobjTransform),
	acceleration(vec3(0.0f,0.0f,0.0f)),
	velocity(vec3(0.0f,0.0f,0.0f)),
	deltaTime(0.0f),
	lastTime(0.0f)
	{}

	void initComponent();

	void execute();

	void endComponent();

	void keyboardInput();

};

