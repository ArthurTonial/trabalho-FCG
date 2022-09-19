#pragma once
#ifndef _SHITMESHSHIT_
#define _SHITMESHSHIT_

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

struct Mesh {
	GLfloat* vertex_position_data;
	GLfloat* vertex_color_data;
	GLfloat* vertex_normal_data;
	GLuint* index_data;
	int n_idx; // number of face indexes
	int n_vert; // number of face indexes

	void addNormal(GLuint vert, glm::vec3 vec);
	void computeNormals();

	Mesh() :
		vertex_position_data(nullptr),
		vertex_color_data(nullptr),
		vertex_normal_data(nullptr),
		index_data(nullptr),
		n_idx(0),
		n_vert(0)
	{
	}
	Mesh(string path);

	void setPositions(GLfloat* v_pos, int n) {
		vertex_position_data = (GLfloat*)malloc(sizeof(GLfloat) * n * 3);
		memcpy(vertex_position_data, v_pos, sizeof(GLfloat) * n * 3);
	}
	void setIndexes(GLuint* v_idx, int n) {
		index_data = (GLuint*)malloc(sizeof(GLuint) * n * 3);
		memcpy(index_data, v_idx, sizeof(GLuint) * n * 3);
	}
};

#endif

