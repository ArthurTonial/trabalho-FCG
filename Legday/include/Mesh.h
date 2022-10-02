#pragma once
#ifndef _SHITMESHSHIT_
#define _SHITMESHSHIT_

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <tiny_obj_loader.h>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

struct Mesh {
	vector<GLuint> indices;
	vector<float>  model_coefficients;
	vector<float>  normal_coefficients;
	vector<float>  texture_coefficients;

	tinyobj::attrib_t                 attrib;
	std::vector<tinyobj::shape_t>     shapes;
	std::vector<tinyobj::material_t>  materials;

	size_t first_index;
	size_t num_triangles;

	float minval;
	float maxval;

	glm::vec3 bbox_min;
	glm::vec3 bbox_max;

	Mesh(const char* filename, const char* basepath = NULL, bool triangulate = true){
		printf("Carregando modelo \"%s\"... ", filename);

		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, basepath, triangulate);

		if (!err.empty())
			fprintf(stderr, "\n%s\n", err.c_str());

		if (!ret)
			throw std::runtime_error("Erro ao carregar modelo.");

		if (shapes.size() > 1) {
			printf("Model path %s has more than one shape\n", filename);
			return;
		}

		first_index = indices.size();
		num_triangles = shapes[0].mesh.num_face_vertices.size();

		minval = std::numeric_limits<float>::min();
		maxval = std::numeric_limits<float>::max();

		bbox_min = glm::vec3(maxval, maxval, maxval);
		bbox_max = glm::vec3(minval, minval, minval);

		for (size_t triangle = 0; triangle < num_triangles; ++triangle)
		{
			assert(shapes[0].mesh.num_face_vertices[triangle] == 3);

			for (size_t vertex = 0; vertex < 3; ++vertex)
			{
				tinyobj::index_t idx = shapes[0].mesh.indices[3 * triangle + vertex];

				indices.push_back(first_index + 3 * triangle + vertex);

				const float vx = attrib.vertices[3 * idx.vertex_index + 0];
				const float vy = attrib.vertices[3 * idx.vertex_index + 1];
				const float vz = attrib.vertices[3 * idx.vertex_index + 2];
				//printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
				model_coefficients.push_back(vx); // X
				model_coefficients.push_back(vy); // Y
				model_coefficients.push_back(vz); // Z
				model_coefficients.push_back(1.0f); // W

				bbox_min.x = std::min(bbox_min.x, vx);
				bbox_min.y = std::min(bbox_min.y, vy);
				bbox_min.z = std::min(bbox_min.z, vz);
				bbox_max.x = std::max(bbox_max.x, vx);
				bbox_max.y = std::max(bbox_max.y, vy);
				bbox_max.z = std::max(bbox_max.z, vz);

				// Inspecionando o código da tinyobjloader, o aluno Bernardo
				// Sulzbach (2017/1) apontou que a maneira correta de testar se
				// existem normais e coordenadas de textura no ObjModel é
				// comparando se o índice retornado é -1. Fazemos isso abaixo.

				if (idx.normal_index != -1)
				{
					const float nx = attrib.normals[3 * idx.normal_index + 0];
					const float ny = attrib.normals[3 * idx.normal_index + 1];
					const float nz = attrib.normals[3 * idx.normal_index + 2];
					normal_coefficients.push_back(nx); // X
					normal_coefficients.push_back(ny); // Y
					normal_coefficients.push_back(nz); // Z
					normal_coefficients.push_back(0.0f); // W
				}

				if (idx.texcoord_index != -1)
				{
					const float u = attrib.texcoords[2 * idx.texcoord_index + 0];
					const float v = attrib.texcoords[2 * idx.texcoord_index + 1];
					texture_coefficients.push_back(u);
					texture_coefficients.push_back(v);
				}
			}
		}

		ComputeNormals(this);

		printf("OK.\n");
	}

	void ComputeNormals(Mesh* model)
	{
		if (!model->attrib.normals.empty())
			return;

		// Primeiro computamos as normais para todos os TRIÂNGULOS.
		// Segundo, computamos as normais dos VÉRTICES através do método proposto
		// por Gouraud, onde a normal de cada vértice vai ser a média das normais de
		// todas as faces que compartilham este vértice.

		size_t num_vertices = model->attrib.vertices.size() / 3;

		std::vector<int> num_triangles_per_vertex(num_vertices, 0);
		std::vector<glm::vec3> vertex_normals(num_vertices, glm::vec3(0.0f, 0.0f, 0.0f));

		for (size_t shape = 0; shape < model->shapes.size(); ++shape)
		{
			size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

			for (size_t triangle = 0; triangle < num_triangles; ++triangle)
			{
				assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

				glm::vec3  vertices[3];
				for (size_t vertex = 0; vertex < 3; ++vertex)
				{
					tinyobj::index_t idx = model->shapes[shape].mesh.indices[3 * triangle + vertex];
					const float vx = model->attrib.vertices[3 * idx.vertex_index + 0];
					const float vy = model->attrib.vertices[3 * idx.vertex_index + 1];
					const float vz = model->attrib.vertices[3 * idx.vertex_index + 2];
					vertices[vertex] = glm::vec3(vx, vy, vz);
				}

				const glm::vec3  a = vertices[0];
				const glm::vec3  b = vertices[1];
				const glm::vec3  c = vertices[2];

				const glm::vec3  n = glm::cross(b - a, c - a);

				for (size_t vertex = 0; vertex < 3; ++vertex)
				{
					tinyobj::index_t idx = model->shapes[shape].mesh.indices[3 * triangle + vertex];
					num_triangles_per_vertex[idx.vertex_index] += 1;
					vertex_normals[idx.vertex_index] += n;
					model->shapes[shape].mesh.indices[3 * triangle + vertex].normal_index = idx.vertex_index;
				}
			}
		}

		model->attrib.normals.resize(3 * num_vertices);
		printf("%d\n", (int)vertex_normals.size());
		for (size_t i = 0; i < vertex_normals.size(); ++i)
		{
			glm::vec3 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
			n = glm::normalize(n);
			if (i < 3) printf("%f %f %f\n", n.x, n.y, n.z);
			model->attrib.normals[3 * i + 0] = n.x;
			model->attrib.normals[3 * i + 1] = n.y;
			model->attrib.normals[3 * i + 2] = n.z;
		}

		for (size_t triangle = 0; triangle < num_triangles; ++triangle)
		{
			assert(shapes[0].mesh.num_face_vertices[triangle] == 3);

			for (size_t vertex = 0; vertex < 3; ++vertex)
			{
				tinyobj::index_t idx = shapes[0].mesh.indices[3 * triangle + vertex];

				if (idx.normal_index != -1)
				{
					const float nx = attrib.normals[3 * idx.normal_index + 0];
					const float ny = attrib.normals[3 * idx.normal_index + 1];
					const float nz = attrib.normals[3 * idx.normal_index + 2];
					normal_coefficients.push_back(nx); // X
					normal_coefficients.push_back(ny); // Y
					normal_coefficients.push_back(nz); // Z
					normal_coefficients.push_back(0.0f); // W
				}

			}
		}
	}
};

#endif

