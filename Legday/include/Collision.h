#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "BaseObject.h"
#include "BaseComponent.h"
#include "camera.h"
#include "Renderer.h"
#include "Scene.h"

using namespace glm;

struct Ray {
	vec3 direction;
	vec3 start;
	Ray() :
		direction(vec3(0.0,0.0,1.0)),
		start(vec3(0.0,0.0,0.0)){}

	Ray(vec3 o, vec3 dir) :
		direction(dir),
		start(o){}
};

struct ColAttribs {
	float l;
	vec3 n;
};

struct CollisionAABB {
	unsigned int num_triangles;
	vec3 bbox_min;
	vec3 bbox_max;
	vector<int> triangles;
	vector<CollisionAABB*> childs;
};

struct CollisionObject {
	CollisionAABB* root_aabb;
	Mesh* mesh_data;
	Transform transform;

	CollisionObject() : mesh_data(nullptr) {}

	CollisionObject(Mesh* ms, Transform tr) : mesh_data(ms), transform(tr) {
		float minval;
		float maxval;

		glm::vec3 bbox_min;
		glm::vec3 bbox_max;

		minval = std::numeric_limits<float>::min();
		maxval = std::numeric_limits<float>::max();

		bbox_min = glm::vec3(maxval, maxval, maxval);
		bbox_max = glm::vec3(minval, minval, minval);

		vector<int> triangles_inside;

		for (int i = 0; i < mesh_data->num_triangles; i++) triangles_inside.push_back(i);
		
		for (size_t triangle = 0; triangle < mesh_data->num_triangles; ++triangle)
		{
			assert(mesh_data->shapes[0].mesh.num_face_vertices[triangle] == 3);

			for (size_t vertex = 0; vertex < 3; ++vertex)
			{
				tinyobj::index_t idx = mesh_data->shapes[0].mesh.indices[3 * triangle + vertex];

				const float vx = mesh_data->attrib.vertices[3 * idx.vertex_index + 0];
				const float vy = mesh_data->attrib.vertices[3 * idx.vertex_index + 1];
				const float vz = mesh_data->attrib.vertices[3 * idx.vertex_index + 2];
				
				vec3 v = vec3(vx, vy, vz);
				vec3 p0 = tr.rotation * (tr.scale * v) + tr.position;

				
				bbox_min.x = std::min(bbox_min.x, p0.x);
				bbox_min.y = std::min(bbox_min.y, p0.y);
				bbox_min.z = std::min(bbox_min.z, p0.z);
				bbox_max.x = std::max(bbox_max.x, p0.x);
				bbox_max.y = std::max(bbox_max.y, p0.y);
				bbox_max.z = std::max(bbox_max.z, p0.z);
			}
		}
		
		root_aabb = build(bbox_min, bbox_max, triangles_inside);
	
	}

	void printHierarchy() {
		printHierarchyAux(root_aabb);
	}

	ColAttribs testCollisionRay(Ray r) {
		return dfsRayAABBtest(root_aabb, r, 0);
	}
	
	CollisionAABB* build(vec3 bb_min, vec3 bb_max, vector<int>& triangles_inside) {
		CollisionAABB* cur = new CollisionAABB();

		cur->bbox_min = bb_min;
		cur->bbox_max = bb_max;
		cur->num_triangles = triangles_inside.size();

		if (cur->num_triangles <= 100) {
			cur->triangles = triangles_inside;
			return cur;
		}

		float xl = bb_max.x - bb_min.x;
		float yl = bb_max.y - bb_min.y;
		float zl = bb_max.z - bb_min.z;

		float x[3] = {
			bb_min.x,
			bb_min.x + (xl / 2.0f),
			bb_max.x
		};
		float y[3] = {
			bb_min.y,
			bb_min.y + (yl / 2.0f),
			bb_max.y
		};
		float z[3] = {
			bb_min.z,
			bb_min.z + (zl / 2.0f),
			bb_max.z
		};
	
		// layer
		for (int layer = 0; layer < 2; layer++) {
			// quadrant
			// row
			for (int i = 0; i < 2; i++) {
				//collum
				for (int j = 0; j < 2; j++) {
					vec3 next_bb_min = vec3(x[i],	y[layer],	z[j]);
					vec3 next_bb_max = vec3(x[i+1],	y[layer+1],	z[j+1]);
					vector<int> next_triangles_inside = trianglesInsideAABB(triangles_inside, next_bb_min, next_bb_max);

					if (next_triangles_inside.size() > 0) {
						cur->childs.push_back(build(next_bb_min, next_bb_max, next_triangles_inside));
					}
				}
			}
		}

		return cur;
	}
	
	private: 
	
	void printHierarchyAux(CollisionAABB* cur, int depth = 0) {
		//printf("depth: %d\n", depth);
		//printf("bb_min: %f %f %f\n", cur->bbox_min.x, cur->bbox_min.y, cur->bbox_min.z);
		//printf("bb_max: %f %f %f\n", cur->bbox_max.x, cur->bbox_max.y, cur->bbox_max.z);

		Renderer::drawAABB(*Scene::mainCamera, cur->bbox_min, cur->bbox_max);

		if (depth > 1) return;

		if (cur->childs.size() > 0) {
			for (int i = 0; i < cur->childs.size(); i++) {
				printHierarchyAux(cur->childs[i], depth + 1);
			}
		}


	}

	bool isInsideAABB(vec3 p, vec3 bb_min, vec3 bb_max) {
		if (p.x >= bb_min.x and p.y >= bb_min.y and p.z >= bb_min.z and
			p.x <= bb_max.x and p.y <= bb_max.y and p.z <= bb_max.z) {
			return true;
		}
		return false;
	}

	vector<int> trianglesInsideAABB(vector<int>& triangles, vec3 bb_min, vec3 bb_max) {
		vector<int> r;

		for (size_t i = 0; i < triangles.size(); ++i)
		{
			size_t triangle = triangles[i];
			assert(this->mesh_data->shapes[0].mesh.num_face_vertices[triangle] == 3);

			vec3 vertices[3];
			for (size_t vertex = 0; vertex < 3; ++vertex)
			{
				tinyobj::index_t idx = this->mesh_data->shapes[0].mesh.indices[3 * triangle + vertex];
				const float vx = this->mesh_data->attrib.vertices[3 * idx.vertex_index + 0];
				const float vy = this->mesh_data->attrib.vertices[3 * idx.vertex_index + 1];
				const float vz = this->mesh_data->attrib.vertices[3 * idx.vertex_index + 2];
				vertices[vertex] = vec3(vx, vy, vz);
			}

			vec3 p0 = transform.rotation * (transform.scale * vertices[0]) + transform.position;
			vec3 p1 = transform.rotation * (transform.scale * vertices[1]) + transform.position;
			vec3 p2 = transform.rotation * (transform.scale * vertices[2]) + transform.position;

			if (isInsideAABB(p0, bb_min, bb_max) or
				isInsideAABB(p1, bb_min, bb_max) or
				isInsideAABB(p2, bb_min, bb_max)) {
				r.push_back(triangle);
			}
		}

		return r;
	}

	ColAttribs testSubMeshCollision(vector<int>& triangles, Ray r) {

		bool hit = false;
		ColAttribs aux;
		aux.l = std::numeric_limits<float>::max();

		for (size_t i = 0; i < triangles.size(); ++i)
		{
			size_t triangle = triangles[i];
			assert(mesh_data->shapes[0].mesh.num_face_vertices[triangle] == 3);

			vec3 vertices[3];
			for (size_t vertex = 0; vertex < 3; ++vertex)
			{
				tinyobj::index_t idx = mesh_data->shapes[0].mesh.indices[3 * triangle + vertex];
				const float vx = mesh_data->attrib.vertices[3 * idx.vertex_index + 0];
				const float vy = mesh_data->attrib.vertices[3 * idx.vertex_index + 1];
				const float vz = mesh_data->attrib.vertices[3 * idx.vertex_index + 2];
				vertices[vertex] = vec3(vx, vy, vz);
			}

			const vec3  v0 = vertices[0];
			const vec3  v1 = vertices[1];
			const vec3  v2 = vertices[2];

			//printf("%f %f %f\n", v0.x, v0.y, v0.z);


			//vec3 p0 = vec3(tr.GetModelMatrix()* vec4(v0, 1.0f));
			//vec3 p1 = vec3(tr.GetModelMatrix()* vec4(v1, 1.0f));
			//vec3 p2 = vec3(tr.GetModelMatrix()* vec4(v2, 1.0f));
			vec3 p0 = transform.rotation * (transform.scale * v0) + transform.position;
			vec3 p1 = transform.rotation * (transform.scale * v1) + transform.position;
			vec3 p2 = transform.rotation * (transform.scale * v2) + transform.position;

			const vec3 n = normalize(cross(p1 - p0, p2 - p0));

			// test if ray is perpendicular to triangle
			if (dot(n, r.direction) == 0.0f) continue;

			float D = -dot(n, p0);

			float t = -(dot(n, r.start) + D) / dot(n, r.direction);

			// test if intersection is behind ray
			if (t < 0.0f) continue;

			// point of intersection betwen ray and triangle's plane 
			vec3 P = r.start + r.direction * t;

			// verify if point is inside triangle
			vec3 C;
			bool inside = true;

			// edge 0
			vec3 edge0 = p1 - p0;
			vec3 vPp0 = P - p0;
			C = cross(edge0, vPp0);
			if (dot(n, C) < 0.0f) inside = false;

			// edge 1
			vec3 edge1 = p2 - p1;
			vec3 vPp1 = P - p1;
			C = cross(edge1, vPp1);
			if (dot(n, C) < 0.0f) inside = false;

			// edge 2
			vec3 edge2 = p0 - p2;
			vec3 vPp2 = P - p2;
			C = cross(edge2, vPp2);
			if (dot(n, C) < 0.0f) inside = false;

			if (!inside) continue;

			hit = true;
			if (aux.l > t) {
				aux.l = t;
				aux.n = n;
			}
		}

		if (!hit) aux.l = -1.0f;
		return aux;
	}

	// o treixo de codigo para testRayAABB foi retirado de uma resposta
	// no gamedev.stackexchange
	// FONTE
	// https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
	bool testRayAABB(Ray r, vec3 bb_min, vec3 bb_max) {
		vec3 dirfrac;
		vec3 lb = bb_min;
		vec3 rt = bb_max;


		// r.dir is unit direction vector of ray
		dirfrac.x = 1.0f / r.direction.x;
		dirfrac.y = 1.0f / r.direction.y;
		dirfrac.z = 1.0f / r.direction.z;
		// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
		// r.org is origin of ray
		float t1 = (lb.x - r.start.x) * dirfrac.x;
		float t2 = (rt.x - r.start.x) * dirfrac.x;
		float t3 = (lb.y - r.start.y) * dirfrac.y;
		float t4 = (rt.y - r.start.y) * dirfrac.y;
		float t5 = (lb.z - r.start.z) * dirfrac.z;
		float t6 = (rt.z - r.start.z) * dirfrac.z;

		float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
		float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

		// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
		if (tmax < 0)
		{
			return false;
		}

		// if tmin > tmax, ray doesn't intersect AABB
		if (tmin > tmax)
		{
			return false;
		}

		return true;
	}

	ColAttribs dfsRayAABBtest(CollisionAABB* cur, Ray r, int depth) {
		ColAttribs collision = {-1.0f, vec3(0.0,0.0,0.0)};
		
		if (!testRayAABB(r, cur->bbox_min, cur->bbox_max)) {
			return collision;
		}

		if (cur->childs.size() == 0) {
			return testSubMeshCollision(cur->triangles, r);
		}

		for (int i = 0; i < cur->childs.size(); i++) {
			ColAttribs childCollision = dfsRayAABBtest(cur->childs[i], r, depth+1);
			if (childCollision.l > 0.0f) {
				if (collision.l < 0.0f or collision.l > childCollision.l) {
					//Renderer::drawAABB(*Scene::mainCamera, cur->bbox_min, cur->bbox_max, vec4(0.5,0.5,1.0,1.0) / (float)depth);
					collision = childCollision;
				}
			}
		}

		return collision;
	}
};

class Collision
{
public:

	static ColAttribs testFloorCollision(Ray r) {
		if (dot(vec3(0.0, 1.0, 0.0), r.direction) == 0.0f) return {-1.0f, vec3(0.0,0.0,0.0)};
		ColAttribs a = { -r.start.y / r.direction.y , vec3(0.0,1.0,0.0) };
		return a;
	}

	static ColAttribs testMeshGenericCollision(Mesh* ms, Transform tr, Ray r) {

		bool hit = false;
		ColAttribs aux;
		aux.l = std::numeric_limits<float>::max();

		for (size_t triangle = 0; triangle < ms->num_triangles; ++triangle)
		{
			assert(ms->shapes[0].mesh.num_face_vertices[triangle] == 3);

			vec3 vertices[3];
			for (size_t vertex = 0; vertex < 3; ++vertex)
			{
				tinyobj::index_t idx = ms->shapes[0].mesh.indices[3 * triangle + vertex];
				const float vx = ms->attrib.vertices[3 * idx.vertex_index + 0];
				const float vy = ms->attrib.vertices[3 * idx.vertex_index + 1];
				const float vz = ms->attrib.vertices[3 * idx.vertex_index + 2];
				vertices[vertex] = vec3(vx, vy, vz);
			}

			const vec3  v0 = vertices[0];
			const vec3  v1 = vertices[1];
			const vec3  v2 = vertices[2];

			vec3 p0 = tr.rotation * (tr.scale * v0) + tr.position;
			vec3 p1 = tr.rotation * (tr.scale * v1) + tr.position;
			vec3 p2 = tr.rotation * (tr.scale * v2) + tr.position;

			const vec3 n = normalize(cross(p1 - p0, p2 - p0));
			
			// o treixo de codigo abaixo foi adaptado de um artigo sobre ray-tracing
			// onde o auto descreve o calculo de interscção de um raio com triangulos
			// FONTE
			// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution

			// test if ray is perpendicular to triangle
			if (dot(n, r.direction) == 0.0f) continue;

			float D = -dot(n, p0);

			float t = -(dot(n, r.start) + D) / dot(n, r.direction);

			// test if intersection is behind ray
			if (t < 0.0f) continue;

			// point of intersection betwen ray and triangle's plane 
			vec3 P = r.start + r.direction * t;

			// verify if point is inside triangle
			vec3 C;
			bool inside = true;

			// edge 0
			vec3 edge0 = p1 - p0;
			vec3 vPp0 = P - p0;
			C = cross(edge0, vPp0);
			if (dot(n, C) < 0.0f) inside = false;

			// edge 1
			vec3 edge1 = p2 - p1;
			vec3 vPp1 = P - p1;
			C = cross(edge1, vPp1);
			if (dot(n, C) < 0.0f) inside = false;

			// edge 2
			vec3 edge2 = p0 - p2;
			vec3 vPp2 = P - p2;
			C = cross(edge2, vPp2);
			if (dot(n, C) < 0.0f) inside = false;

			if (!inside) continue;

			hit = true;
			if (aux.l > t) {
				aux.l = t;
				aux.n = n;
			}
		}

		if (!hit) aux.l = -1.0f;
		return aux;
	}

	static ColAttribs testSphereSphereCollision(float radiusA, Transform trA, float radiusB, Transform trB) {
		vec3 ab = trA.position - trB.position;
		if (length(ab) > radiusA + radiusB) {
			return {-1.0f, vec3(0.0,0.0,0.0)};
		}
		else {
			return { radiusA + radiusB - length(ab), normalize(ab)};
		}
	}

	static ColAttribs testSphereAABBCollision(float radiusA, Transform trA, vec3 bb_min, vec3 bb_max) {
		
		vec3 closest_point = closestPointAABB(trA.position, bb_min, bb_max);
		vec3 ab = trA.position - closest_point;
		
		if (length(ab) > radiusA) {
			return { -1.0f, vec3(0.0,0.0,0.0) };
		}
		else {
			return { radiusA - length(ab), normalize(ab) };
		}
	}

	static ColAttribs testSphereAABBCollision(float radiusA, Transform trA, CollisionAABB aabb) {

		vec3 bb_min = aabb.bbox_min;
		vec3 bb_max = aabb.bbox_max;

		vec3 closest_point = closestPointAABB(trA.position, bb_min, bb_max);
		vec3 ab = trA.position - closest_point;

		if (length(ab) > radiusA) {
			return { -1.0f, vec3(0.0,0.0,0.0) };
		}
		else {
			return { radiusA - length(ab), normalize(ab) };
		}
	}

private: 
	static vec3 closestPointAABB(vec3 p, vec3 bb_min, vec3 bb_max) {
		return vec3(
			std::min(bb_max.x, std::max(bb_min.x, p.x)),
			std::min(bb_max.y, std::max(bb_min.y, p.y)),
			std::min(bb_max.z, std::max(bb_min.z, p.z))
		);
	}

};