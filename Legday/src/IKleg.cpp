#include "IKleg.h"

Mesh Segment::midSegmentMesh = Mesh("meshes/legMid.obj");
Mesh Segment::endSegmentMesh = Mesh("meshes/legTip2.obj");
Shader Segment::LegShader;

void Segment::drawSegment() {
	body.runComponents();
	if (child != NULL) {
		child->drawSegment();
	}
}

void Segment::update(vec3 pos, vec3 target, vec3 up, float legLength) {
	
	vec3 t = (target - pos);
	vec3 upVec = normalize(pos - up);
	float segLen = len * size;
	float f = (segLen / legLength);
	
	float h = segLen;
	float a = length(t) * f;
	float b = sqrt(h * h - a * a);

	transform->position = pos;
	transform->lookAt(t, upVec);

	if (b > 0.003) {
		transform->lookAt(t * f + transform->getUp() * b, transform->getUp());
	}

	if (child != NULL) {
		child->update(transform->position + transform->getFront() * segLen, target, up, legLength - segLen);
	}
}

void IKleg::build() {
	
	int temp = nSegs;
	for (int i = 0; i < temp; i++) {
		addLeg(2.66f);
	}

	update();
}

void IKleg::addLeg(float len) {
	legLength += len;

	if (root == NULL) {
		root = new Segment(0, len, 1.0f, NULL, 1);

		nSegs = 1;

		return;
	}
	if (nSegs == 1) {
		float lastLen = root->len;
		delete(root);

		root = new Segment(0, lastLen, 1.0f, NULL, 0);
		root->child = new Segment(1, len, 1.0f, root, 1);

		nSegs = 2;

		return;
	}

	Segment* cur = root;												//            \/ i'm here
	while (cur->child != NULL)	cur = cur->child;						//  . . . ->0->1
																		//            /\ save len from here
	float lastLen = cur->len;											//            
																		//         \/ now i'm here
	cur = cur->parent;													//  . . . ->0->1
	delete(cur->child);													//  . . . ->0  delete last one
																		//             
	cur->child = new Segment(nSegs - 1, lastLen, 1.0f, cur, 0);			//  . . . ->0->0 new child here 
																		//             
	cur = cur->child;													//            \/ now i'm here
																		//  . . . ->0->0
	cur->child = new Segment(nSegs, len, 1.0f, cur, 1);					//               
																		//  . . . ->0->0->1 new child here
	nSegs++;															//  update number of segs
}

void IKleg::removeLeg(int idx) {
	if (idx == -1) idx = nSegs - 1;
	if (nSegs < 3) return;

	int i = 0;
	Segment* cur = root;
	while (i != idx) {
		cur = cur->child;
		i++;
	}

	legLength -= cur->len;

	if (i == nSegs - 1) {
		cur = cur->parent;
		delete cur->child;
		float lastLen = cur->len;
		cur = cur->parent;
		delete cur->child;
		cur->child = new Segment(i - 1, lastLen, 1.0f, cur, 1);
	}
	else {
		delete cur;
	}

	nSegs--;
}

void IKleg::draw() {
	root->drawSegment();
}

void IKleg::update() {
	
	float distFromCur = length(target - getTrueTransform().position);
	vec3 t = normalize(target - getTrueTransform().position) * std::min(distFromCur, legLength);
	
	float rootSz = (root->len * root->size);

	vec3 up = (cross(normalize(t), cross(vec3(0.0,-1.0,0.0), normalize(t)))) + t / 2.0f;
	up = 0.1f * (cross(normalize(t), cross(vec3(0.0, -1.0, 0.0), normalize(t)))) + t / 2.0f;

	root->update(getTrueTransform().position, t + getTrueTransform().position, up + getTrueTransform().position, legLength);
}

void IKleg::initComponent() {
	build();
}

void IKleg::execute() {
	update();
	draw();
}