#pragma once
#include "BaseObject.h"
#include "Collision.h"
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
	bool debugMode;
	float t_dist;
	float follow_dist;
	bool controlCamera;
	float hieght;
	MeshRenderer* mr;
	PlayerController* pc;
	vector<vec3> curLegPos;
	vector<vec3> LegTarget[4];
	vector<legInterpolationState> legState;

	vec3 wantDir;

	SpiderTank(unsigned int oid, int nLegs) : BaseObject(oid),
		nLegs(nLegs),
		debugMode(false),
		deltaTime(-1.0f),
		t_dist(2.5f),
		follow_dist(1.7f),
		lastTime(-1.0f),
		interpSpeed(10.0f),
		hieght(0.5f),
		upLegs(0b1111),
		wantDir(vec3(0.0f,0.0f,0.0f)),
		curLegPos(vector<vec3>(4)),
		LegTarget(),
		legState(vector<legInterpolationState>(4))
	{
		for (int i = 0; i < 4; i++) LegTarget[i].resize(3);

		Mesh* bodyMesh = new Mesh("meshes/sphere.obj");
		Shader* bodyShader = new Shader("shaders/pbr.vs", "shaders/sphereShader.fs");

		mr = new MeshRenderer(oid, 0, &transform);
		pc = new PlayerController(oid, 1, &transform);

		addComponent(mr);
		mr->setMesh(bodyMesh);
		mr->setMaterial(new Material(1.0f, 80.0f, 0.0f, vec4(0.5f, 0.5f, 0.5f,1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), bodyShader));
		mr->componentTransform = Transform(vec3(0.0), quat(1.0, vec3(0.0)), vec3(1.0, 1.0, 1.0));
		mr->componentTransform.lookAt(vec3(0.0, 0.0, 1.0));

		addComponent(pc);
	}

	void resetShader() {
		printf("opa\n");
		mr->objectMaterial->sh = new Shader("shaders/pbr.vs", "shaders/sphereShader.fs");
		mr->initComponent();
	}

	void init() {
	}

	void updateDirectionWithCamera() {
		wantDir = normalize(vec3(Scene::mainCamera->Front.x, 0.0f, Scene::mainCamera->Front.z));
	}

	void run() {

		pc->is_active = controlCamera;

		if (lastTime > 0.0f) {
			deltaTime = glfwGetTime() - lastTime;

			if (controlCamera) {
				updateDirectionWithCamera();
			}

			vec3 avgNormal = averageNormal();

			// compute and interpolates wanted height with ray casting collision testing
			Ray r(transform.position, -avgNormal);

			//ColAttribs c = Collision::testMeshCollision(((CubeObject*)Scene::objects[1])->objectMesh, Scene::objects[1]->transform, r);
			ColAttribs c = ((CubeObject*)Scene::objects[1])->objectCollision.testCollisionRay(r);
			ColAttribs cFloor = Collision::testFloorCollision(r);
			float l = c.l;
			vec3 n = c.n;

			if (c.l < 0.0f or cFloor.l <= c.l) {
				l = cFloor.l;
				n = cFloor.n;
				if(debugMode) Renderer::drawLine(*Scene::mainCamera, r.direction, r.start, 1000.0f, vec4(0.0, 0.0, 1.0, 1.0));
			}
			else {
				if (debugMode)Renderer::drawLine(*Scene::mainCamera, r.direction, r.start, l, vec4(0.0, 1.0, 0.0, 1.0));
			}

			if (l > 0.0f and l < 5.0f) {
				vec3 p = r.start + r.direction * l;

				vec3 wantHeight = p + avgNormal * 2.0f;
				vec3 forceHeightVec = normalize(wantHeight - transform.position);
				float forceHeightMag = std::min(2.0f, length(transform.position - wantHeight) * turnSpeed * 0.5f);

				//transform.position = transform.position + forceHeightVec * forceHeightMag * deltaTime;
				transform.position = wantHeight;
			}
			
			//
			vec3 forceVec = normalize(transform.getFront() - wantDir);
			float forceMag = std::min(20.0f, length(transform.getFront() - wantDir) + turnSpeed);

			transform.lookAt(transform.getFront() + forceVec * forceMag * deltaTime, avgNormal);

			if (debugMode)Renderer::drawLine(*Scene::mainCamera, averageNormal(), transform.position, 5.0f, vec4(1.0, 0.0, 1.0, 1.0));

			// test cube colision

			ColAttribs cubeCol = Collision::testSphereAABBCollision(5.0f, transform, *((CubeObject*)Scene::objects[2])->objectCollision.root_aabb);
			if (cubeCol.l >= 0.0f) {
				transform.position += (cubeCol.l * cubeCol.n);
			}

			// test sphere colision

			ColAttribs sphereCol = Collision::testSphereSphereCollision(5.0f, transform, 10.0f, ((CubeObject*)Scene::objects[3])->transform);
			if (sphereCol.l >= 0.0f) {
				transform.position += (sphereCol.l * sphereCol.n);
			}

			updateLegs();
		}
		lastTime = glfwGetTime();
	}

	vec3 averageNormal() {
		vec3 n = vec3(0.0f);
		// normal 0 - 1 - 2
		n += normalize(cross(curLegPos[0] - curLegPos[1], curLegPos[0] - curLegPos[2]));
		// normal 0 - 1 - 3
		n += normalize(cross(curLegPos[0] - curLegPos[1], curLegPos[0] - curLegPos[3]));
		// normal 0 - 2 - 3
		n += normalize(cross(curLegPos[0] - curLegPos[2], curLegPos[0] - curLegPos[3]));
		// normal 1 - 2 - 3
		n += normalize(cross(curLegPos[1] - curLegPos[2], curLegPos[1] - curLegPos[3]));

		n = normalize(n);

		return -n;
	}

	void build(int nSegs) {
		// r legs
		
		if (getComponents().size() > 2) {
			printf("opppaa\n");
			getComponents()[2] = new IKleg(getoid(), 2, nSegs, &transform);
			getComponents()[3] = new IKleg(getoid(), 3, nSegs, &transform);
			getComponents()[4] = new IKleg(getoid(), 4, nSegs, &transform);
			getComponents()[5] = new IKleg(getoid(), 5, nSegs, &transform);
		}
		else {
			addComponent(new IKleg(getoid(), 2, nSegs, &transform));
			addComponent(new IKleg(getoid(), 3, nSegs, &transform));
			addComponent(new IKleg(getoid(), 4, nSegs, &transform));
			addComponent(new IKleg(getoid(), 5, nSegs, &transform));
		

			components[2]->componentTransform.position = vec3(1.0,0.0, 1.0);
			components[3]->componentTransform.position = vec3(-1.0,0.0, 1.0);
			components[4]->componentTransform.position = vec3(-1.0,0.0,-1.0);
			components[5]->componentTransform.position = vec3(1.0,0.0,-1.0);

			/*
			*
			*	0     1
			*    \ z /
			*    x-'
			*    /   \
			*   2     3
			* 
			*/

			for (int i = 0; i < 4; i++) {
				float of = i * pi<float>() / 2.0f;
				float x = cos(pi<float>() / 4.0f + of);
				float z = sin(pi<float>() / 4.0f + of);

				curLegPos[i] = vec3(x, -0.5, z);
			}

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
	}

	void updateLegs() {

		float of_[4] = { 0.0f, 0.0f, -pi<float>() / 8.0f, pi<float>() / 8.0f };

		for (int i = 0; i < 4; i++) {
			float of = i * pi<float>() / 2.0f;
			float x = cos(pi<float>() / 4.0f + of + of_[i]);
			float z = sin(pi<float>() / 4.0f + of + of_[i]);

			LegTarget[i][0] = transform.rotation * normalize(vec3(x, -t_dist * 0.5f, z));
			LegTarget[i][1] = transform.rotation * normalize(vec3(x, -t_dist, z));
			LegTarget[i][2] = transform.rotation * normalize(vec3(x, -t_dist * 2.0f, z));
		}
		
		for (int i = 0; i < 4; i++) {

			vec3 r_o = ((IKleg*)components[i+2])->getTrueTransform().position + vec3(0.0f,2.0f,0.0f);

			Ray rays[3];
			for (int r = 0; r < 3; r++) {
				rays[r] = Ray(r_o, LegTarget[i][r]);
			}

			ColAttribs colisions[3] = {
				{-1.0f, vec3(0.0)},
				{-1.0f, vec3(0.0)},
				{-1.0f, vec3(0.0)}
			};
			for (int c = 0; c < 3; c++) {
				colisions[c] = ((CubeObject*)Scene::objects[1])->objectCollision.testCollisionRay(rays[c]);
				
				ColAttribs cFloor = Collision::testFloorCollision(rays[c]);
				if (colisions[c].l < 0.0f or (cFloor.l <= colisions[c].l and cFloor.l > 0.0f)) {
					colisions[c] = cFloor;
					if (debugMode)Renderer::drawLine(*Scene::mainCamera, rays[c].direction, rays[c].start, 1000.0f, vec4(0.0, 0.0, 1.0, 1.0));
				}
				else {
					if (debugMode)Renderer::drawLine(*Scene::mainCamera, rays[c].direction, rays[c].start, colisions[c].l, vec4(0.0, 1.0, 0.0, 1.0));
				}
			}

			//ColAttribs c = Collision::testMeshCollision(((CubeObject*)Scene::objects[1])->objectMesh, Scene::objects[1]->transform, r);

			float l;

			vec3 target0 = rays[0].start + rays[0].direction * colisions[0].l;
			vec3 target1 = rays[1].start + rays[1].direction * colisions[1].l;
			vec3 target2 = rays[2].start + rays[2].direction * colisions[2].l;
			vec3 target = target0;

			/*
			vec3 legRoot = ((IKleg*)components[i + 2])->getTrueTransform().position;
			float legLen = ((IKleg*)components[i + 2])->legLength;

			if (length(target) > legLen) target = target1;
			if (length(target) > legLen) target = target2;
			*/

			if (legState[i].fetch) interpolateLeg(i, target);
			else if (length(target - curLegPos[i]) > follow_dist and (upLegs & (1 << i))) {
				legState[i].fetch = true;
				legState[i].legInitialPos = curLegPos[i];

				if (i == 0 or i == 2) upLegs = 0b0101;
				else upLegs = 0b1010;
			}

			((IKleg*)components[i + 2])->target = curLegPos[i];
		}
	}

	void interpolateLeg(int i, vec3 target) {
		curLegPos[i] = (target - legState[i].legInitialPos) * legState[i].alphaValue + legState[i].legInitialPos;

		curLegPos[i] += vec3(0.0f, sin(legState[i].alphaValue / 1.4f * PI), 0.0f);

		legState[i].alphaValue += interpSpeed * deltaTime;

		if (legState[i].alphaValue >= 1.0f) {
			legState[i].fetch = false;
			legState[i].alphaValue = 0.0f;
			upLegs = 0b1111;
		}
	}
};
