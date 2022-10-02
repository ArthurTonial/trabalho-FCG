#include "MeshRenderer.h"

void MeshRenderer::initComponent() {
	
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint VBO_model_coefficients_id;
	glGenBuffers(1, &VBO_model_coefficients_id);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
	glBufferData(GL_ARRAY_BUFFER, objectMesh->model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, objectMesh->model_coefficients.size() * sizeof(float), objectMesh->model_coefficients.data());
	GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
	GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
	glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(location);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (!objectMesh->normal_coefficients.empty())
	{
		GLuint VBO_normal_coefficients_id;
		glGenBuffers(1, &VBO_normal_coefficients_id);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
		glBufferData(GL_ARRAY_BUFFER, objectMesh->normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, objectMesh->normal_coefficients.size() * sizeof(float), objectMesh->normal_coefficients.data());
		location = 1; // "(location = 1)" em "shader_vertex.glsl"
		number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
		glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(location);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (!objectMesh->texture_coefficients.empty())
	{
		GLuint VBO_texture_coefficients_id;
		glGenBuffers(1, &VBO_texture_coefficients_id);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
		glBufferData(GL_ARRAY_BUFFER, objectMesh->texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, objectMesh->texture_coefficients.size() * sizeof(float), objectMesh->texture_coefficients.data());
		location = 2; // "(location = 1)" em "shader_vertex.glsl"
		number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
		glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(location);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	GLuint indices_id;
	glGenBuffers(1, &indices_id);

	// "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, objectMesh->indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, objectMesh->indices.size() * sizeof(GLuint), objectMesh->indices.data());
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
	//

	// "Desligamos" o VAO, evitando assim que operações posteriores venham a
	// alterar o mesmo. Isso evita bugs.
	glBindVertexArray(0);

	renderObj = RenderObject(*objectMaterial, VAO, objectMesh->indices.size(), getTrueTransform());
	renderObj.bbox_min = objectMesh->bbox_min;
	renderObj.bbox_max = objectMesh->bbox_max;
}

void MeshRenderer::execute() {
	renderObj.transform = getTrueTransform();
	Renderer::renderQ.push(&renderObj);
}

void MeshRenderer::endComponent() {
	return;
}
