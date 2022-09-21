#pragma once
#include "CubeObject.h"
#include "BaseComponent.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


#define PI 3.14159265359
#define e 0.0000006f

using namespace glm;

struct Segment {

	int id;
	float len;
	float size;
	BaseObject body;
	Transform* transform;
	Segment* child;
	Segment* parent;

	static Mesh midSegmentMesh;
	static Mesh endSegmentMesh;
	static Shader LegShader;

	Segment(int id, float len, float size, Segment* parent, int type) :
		id(id),
		len(len),
		size(size),
		body(BaseObject(id)),
		transform(nullptr),
		child(NULL),
		parent(parent)
	{
		body.addComponent(new MeshRenderer(body.getoid(), 0, &(body.transform)));
		if (type == 0) {
			((MeshRenderer*)body.getComponents()[0])->setMesh(&midSegmentMesh);
		}
		else if (type == 1) {
			((MeshRenderer*)body.getComponents()[0])->setMesh(&endSegmentMesh);
		}

		((MeshRenderer*)body.getComponents()[0])->setMaterial(new Material(1.0f, 50.0f, 0.0f, vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f)));
		
		((MeshRenderer*)body.getComponents()[0])->componentTransform.scale = vec3(size, size, size);

		body.initComponents();


		transform = &body.transform;
	}

	~Segment(){
		if(parent != NULL) parent->child = child;
	}

	static void drawCube(glm::vec3 pos, float size);

	void drawSegment();

	void update(vec3 pos, vec3 target, vec3 up, float legLength);

	void deleteSegment() {

	}
};

class IKleg : public BaseComponent
{
	Segment* root;

public:
	vec3 target;
	int nSegs;
	float legLength;

	IKleg(int oid, int cid, int nSegs, Transform* myobjTransform) : BaseComponent(oid, cid, myobjTransform),
		nSegs(nSegs),
		legLength(0.0),
		target(vec3(2.5, 0.0, 0.0)),
		root(NULL)
	{
	}
	

	static void drawLine(vec3 a, vec3 b) {
		for (int i = 1; i <= 10; i++) {
			vec3 p = (b - a) * (1.0f * i) / 10.0f + a;
			Segment::drawCube(p, 0.05);
		}
	}

	void build();

	void addLeg(float len);

	void removeLeg(int idx = -1);

	void draw();

	void update();

	void initComponent();

	void execute();

	//void endComponent();
};

