#pragma once
#include "BaseObject.h"
#include "IKleg.h"
#include "MeshRenderer.h"
#include "PlayerController.h"


#define PI 3.14159265359f

using namespace glm;

struct legInterpolationState {
	vec3 legInitialPos;
	float alphaValue;
	bool fetch;
};

class SpiderTank : public BaseObject
{
	int nLegs;
	float interpSpeed;
	float deltaTime;
	float lastTime;
	float turnSpeed = 10.0f;
	int upLegs;
public:
	bool controlCamera;
	float hieght;
	vector<vec3> curLegPos;
	vector<vec3> LegTarget;
	vector<legInterpolationState> legState;

	vec3 wantDir;

	SpiderTank(unsigned int oid, int nLegs) : BaseObject(oid),
		nLegs(nLegs),
		deltaTime(-1.0f),
		lastTime(-1.0f),
		interpSpeed(10.0f),
		hieght(1.5f),
		upLegs(0b1111),
		wantDir(vec3(0.0f,0.0f,0.0f)),
		curLegPos(vector<vec3>(4)),
		LegTarget(vector<vec3>(4)),
		legState(vector<legInterpolationState>(4))
	{
		Mesh* bodyMesh = new Mesh("meshes/sphere.obj");
		Shader* bodyShader = new Shader("shaders/pbr.vs", "shaders/sphereShader.fs");

		addComponent(new MeshRenderer(oid, 0, &transform));
		((MeshRenderer*)getComponents()[0])->setMesh(bodyMesh);
		((MeshRenderer*)getComponents()[0])->setMaterial(new Material(1.0f, 80.0f, 0.0f, vec4(0.5f, 0.5f, 0.5f,1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), bodyShader));
		((MeshRenderer*)getComponents()[0])->componentTransform = Transform(vec3(0.0), quat(1.0, vec3(0.0)), vec3(1.0, 1.0, 1.0));
		((MeshRenderer*)getComponents()[0])->componentTransform.lookAt(vec3(0.0, 0.0, 1.0));

		addComponent(new PlayerController(oid, 1, &transform));
	}

	void resetShader() {
		printf("opa\n");
		((MeshRenderer*)getComponents()[0])->objectMaterial->sh = new Shader("shaders/pbr.vs", "shaders/sphereShader.fs");
		((MeshRenderer*)getComponents()[0])->initComponent();
	}

	void init() {
	}

	void updateDirectionWithCamera() {
		wantDir = normalize(vec3(Scene::mainCamera->Front.x, 0.0f, Scene::mainCamera->Front.z));
	}

	void run() {

		((PlayerController*)getComponents()[1])->is_active = controlCamera;

		if (lastTime > 0.0f) {
			deltaTime = glfwGetTime() - lastTime;

			if (controlCamera) {
				updateDirectionWithCamera();
			}
			vec3 forceVec = normalize(transform.getFront() - wantDir);
			float forceMag = std::min(20.0f, length(transform.getFront() - wantDir) + turnSpeed);

			transform.lookAt(transform.getFront() + forceVec * forceMag * deltaTime);

			updateLegs();
		}
		lastTime = glfwGetTime();
	}

	void build() {
		// r legs
		addComponent(new IKleg(getoid(), 2, 2, &transform));
		addComponent(new IKleg(getoid(), 3, 2, &transform));
		addComponent(new IKleg(getoid(), 4, 2, &transform));
		addComponent(new IKleg(getoid(), 5, 2, &transform));

		components[2]->componentTransform.position = vec3(1.5,0.0,1.5);
		components[3]->componentTransform.position = vec3(-1.5,0.0,1.5);
		components[4]->componentTransform.position = vec3(1.5,0.0,-1.5);
		components[5]->componentTransform.position = vec3(-1.5,0.0,-1.5);

		/*
		*
		*	0     1
		*    \ z /
		*    x-'
		*    /   \
		*   2     3
		* 
		*/

		LegTarget[0] = (components[2]->getTrueTransform().position) - transform.position;
		LegTarget[1] = (components[3]->getTrueTransform().position) - transform.position;
		LegTarget[2] = (components[4]->getTrueTransform().position) - transform.position;
		LegTarget[3] = (components[5]->getTrueTransform().position) - transform.position;

		((IKleg*)components[2])->target = curLegPos[0];
		((IKleg*)components[3])->target = curLegPos[1];
		((IKleg*)components[4])->target = curLegPos[2];
		((IKleg*)components[5])->target = curLegPos[3];

		for (int i = 0; i < 4; i++) {
			legState[i] = {
				curLegPos[i],
				0.0f,
				false
			};
		}

	}

	void updateLegs() {

		for (int i = 0; i < 4; i++) {

			vec3 t = (transform.rotation * LegTarget[i]) * 2.5f + vec3(0.0f, -hieght, 0.0f) + transform.position;
			//Segment::drawCube(t, 0.1f);

			if (legState[i].fetch) interpolateLeg(i);
			else if (length(t - curLegPos[i]) > 1.7f and upLegs & (1 << i)) {
				legState[i].fetch = true;
				legState[i].legInitialPos = curLegPos[i];

				if (i == 0 or i == 3) upLegs = 0b1001;
				else upLegs = 0b0110;
			}

			((IKleg*)components[i + 2])->target = curLegPos[i];
		}
	}

	void interpolateLeg(int i) {
		vec3 target = (transform.rotation * LegTarget[i]) * 2.5f + vec3(0.0f, -hieght, 0.0f) + transform.position;

		curLegPos[i] = (target - legState[i].legInitialPos) * legState[i].alphaValue + legState[i].legInitialPos;

		curLegPos[i] += vec3(0.0f, sin(legState[i].alphaValue / 1.4f * PI), 0.0f);

		legState[i].alphaValue += interpSpeed * deltaTime;

		if (legState[i].alphaValue >= 1.5f) {
			legState[i].fetch = false;
			legState[i].alphaValue = 0.0f;
			upLegs = 0b1111;
		}
	}
};
