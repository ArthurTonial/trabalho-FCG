#pragma once
#include "BaseObject.h"
#include "MeshRenderer.h"

class CubeObject : public BaseObject
{

public:
	CubeObject(unsigned int oid, glm::vec3 offset, glm::vec3 rotation, glm::vec3 scale) : BaseObject(oid)
	{

		GLfloat v_positions[8 * 3] = {
			-1.0,  1.0, -1.0,
			-1.0,  1.0,  1.0,
			 1.0,  1.0, -1.0,
			 1.0,  1.0,  1.0,
			-1.0, -1.0, -1.0,
			-1.0, -1.0,  1.0,
			 1.0, -1.0, -1.0,
			 1.0, -1.0,  1.0
		};

		GLuint v_indexes[12 * 3] = {
			0, 1, 2,
			2, 1, 3,
			3, 7, 2,
			7, 6, 2,
			7, 4, 6,
			7, 5, 4,
			4, 5, 1,
			1, 0, 4,
			4, 0, 6,
			6, 0, 2,
			1, 5, 3,
			3, 5, 7
		};

		for (int i = 0; i < 8; i++) {
			glm::vec4 v = glm::vec4(v_positions[i * 3], v_positions[i * 3 + 1], v_positions[i * 3 + 2], 1.0);

			v = Transform::Matrix_Translate(offset) * Transform::Matrix_Scale(scale) * Transform::Matrix_Rotate_Z(rotation.z) * Transform::Matrix_Rotate_Z(rotation.y) * Transform::Matrix_Rotate_Z(rotation.z) * v;

			v_positions[i * 3] = v.x;
			v_positions[i * 3 + 1] = v.y;
			v_positions[i * 3 + 2] = v.z;
		}

		//Mesh cubeMesh = Mesh("meshes/legMid.obj");

		//this->addComponent(new MeshRenderer(oid, 0, &transform));
		//((MeshRenderer*)getComponents()[0])->setMesh(&cubeMesh);
		//((MeshRenderer*)getComponents()[0])->setMaterial(new Material(1.0f, 1.0f, 0.0f, vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f)));
	}

};

