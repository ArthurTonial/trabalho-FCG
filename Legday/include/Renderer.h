#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>

#include <stb_image.h>

#include "Shader.h"
#include "Compute_Shader.h"
#include "camera.h"
#include "Transform.h"


using namespace std;
using namespace glm;

struct SunLight {
	vec4 color;
	vec3 position;
	vec3 direction;
	vec3 target;
	// euler Angles
	float Yaw;
	float Pitch;

	float debug1;
	float debug2;

	mat4 GetViewMatrix() const {
		return lookAt(position, position+direction, vec3(0.0f, 1.0f, 0.0f));
	}

	glm::mat4 GetProjectionMatrix() const {
		return glm::ortho(-debug2, debug2, -debug2, debug2, 1.0f, debug1);
	}

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= 0.1f;
		yoffset *= 0.1f;

		Yaw -= xoffset;
		Pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateVectors();
	}

	void updateVectors() {
		glm::vec3 pos;
		pos.x = cos(glm::radians(Yaw)) * cos(glm::radians(-Pitch));
		pos.y = sin(glm::radians(-Pitch));
		pos.z = sin(glm::radians(Yaw)) * cos(glm::radians(-Pitch));

		position = pos * 200.0f + target;

		glm::vec3 front = normalize(target - position);

		direction = glm::normalize(front);
		// also re-calculate the Right and Up vector
		//Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		//Up = glm::normalize(glm::cross(Right, Front));
	}
};

struct Material {
	static float time;

	float glossy;
	float diffuse;
	float metallic;
	float selected;

	vec4 diffuse_color;
	vec4 glossy_color;

	Shader* sh;

	Material(float diffuse, float glossy, float metallic, vec4 diffuse_c, vec4 glossy_c, Shader* sh) :
		glossy(glossy),
		diffuse(diffuse),
		metallic(metallic),
		diffuse_color(diffuse_c),
		glossy_color(glossy_c),
		sh(sh)
	{};

	void setShaderOptions(const Camera& camera, const SunLight sun, const Transform transform) {

		sh->setFloat("time", time);

		sh->setFloat("diffuse", diffuse);
		sh->setFloat("glossy", glossy);
		sh->setFloat("metallic", metallic);
		sh->setFloat("selected", selected);
		sh->setFloat4("diffuse_color", diffuse_color);
		sh->setFloat4("glossy_color", glossy_color);

		sh->setFloat4("light_color", sun.color);
		sh->setFloat4("camera_position", vec4(camera.Position, 1.0f));
		sh->setFloat4("camera_direction", vec4(camera.Front, 0.0f));
		sh->setFloat4("sun_position", vec4(sun.position, 1.0f));
		sh->setFloat4("sun_direction", vec4(sun.direction, 0.0f));

		sh->setMatrix4("sun_view_m", sun.GetViewMatrix());
		sh->setMatrix4("sun_proj_m", sun.GetProjectionMatrix());

		sh->setMatrix4("mode_m", transform.GetModelMatrix());
		sh->setMatrix4("view_m", camera.GetViewMatrix());
		sh->setMatrix4("proj_m", camera.GetProjectionMatrix());

		sh->setInt("shadowMap", 0);
		
		for (int i = 1; i < 10; i++) {
			string name = "TextureImage";
			name += to_string(i);

			sh->setInt(name, i);
		}

	}

	void setShaderOptions(const Camera& camera, const Transform transform) {
		sh->use();
		sh->setFloat("time", time);

		sh->setFloat("diffuse", diffuse);
		sh->setFloat("glossy", glossy);
		sh->setFloat("metallic", metallic);
		sh->setFloat("selected", selected);
		sh->setFloat4("diffuse_color", diffuse_color);
		sh->setFloat4("glossy_color", glossy_color);

		sh->setFloat4("light_color", vec4(0.0f));
		sh->setFloat4("camera_position", vec4(camera.Position, 1.0f));
		sh->setFloat4("camera_direction", vec4(camera.Front, 0.0f));
		sh->setFloat4("sun_position", vec4(0.0f));
		sh->setFloat4("sun_direction", vec4(0.0f));

		sh->setMatrix4("sun_view_m", mat4(1.0f));
		sh->setMatrix4("sun_proj_m", mat4(1.0f));

		sh->setMatrix4("mode_m", transform.GetModelMatrix());
		sh->setMatrix4("view_m", camera.GetViewMatrix());
		sh->setMatrix4("proj_m", camera.GetProjectionMatrix());

		sh->setInt("shadowMap", 0);

		for (int i = 1; i < 10; i++) {
			string name = "TextureImage";
			name += to_string(i);

			sh->setInt(name, i);
		}
	}

	void setShaderOptions(const SunLight sun, const Transform transform) {
		sh->use();
		sh->setFloat("time", time);

		sh->setFloat("diffuse", diffuse);
		sh->setFloat("glossy", glossy);
		sh->setFloat("metallic", metallic);
		sh->setFloat("selected", selected);
		sh->setFloat4("diffuse_color", diffuse_color);
		sh->setFloat4("glossy_color", glossy_color);

		sh->setFloat4("camera_position", vec4(sun.position, 1.0f));
		sh->setFloat4("camera_direction", vec4(sun.direction, 0.0f));

		sh->setMatrix4("mode_m", transform.GetModelMatrix());
		sh->setMatrix4("view_m", sun.GetViewMatrix());
		sh->setMatrix4("proj_m", sun.GetProjectionMatrix());

		sh->setMatrix4("sun_view_m", mat4(1.0f));
		sh->setMatrix4("sun_proj_m", mat4(1.0f));
		sh->setFloat4("light_color", vec4(0.0f));
		sh->setFloat4("sun_direction", vec4(0.0f));
		sh->setFloat4("sun_position", vec4(0.0f));

		sh->setInt("shadowMap", 0);

		for (int i = 1; i < 10; i++) {
			string name = "TextureImage";
			name += to_string(i);

			sh->setInt(name, i);
		}
	}
};

struct RenderObject {
	Transform transform;
	Material material;
	GLuint VAO;
	int n_index;

	glm::vec3 bbox_min;
	glm::vec3 bbox_max;

	RenderObject() :
		material(Material(1.0f,1.0f,1.0f,vec4(1.0f), vec4(1.0f), nullptr)),
		VAO(0),
		n_index(0),
		transform(Transform()),
		bbox_min(vec3(0.0)),
		bbox_max(vec3(0.0)) {}

	RenderObject(Material material, GLuint vao, int idx, Transform tr) :
		material(material),
		VAO(vao),
		n_index(idx),
		transform(tr),
		bbox_min(vec3(0.0)),
		bbox_max(vec3(0.0)) {}

	RenderObject(Material material, vector<GLfloat>& v_pos, vector<GLuint>& indices, Transform tr) :
		material(material),
		VAO(0),
		n_index(indices.size()),
		transform(tr),
		bbox_min(vec3(0.0)),
		bbox_max(vec3(0.0)) {
	
		GLuint vertex_array_object_id;
		glGenVertexArrays(1, &vertex_array_object_id);
		glBindVertexArray(vertex_array_object_id);

		// binding positions

		GLuint VBO_NDC_coefficients_id;
		glGenBuffers(1, &VBO_NDC_coefficients_id);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * v_pos.size(), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * v_pos.size() , v_pos.data());


		GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
		GLint  number_of_dimensions = 3; // vec3 em "shader_vertex.glsl"
		glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(location);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// bind indices

		GLuint indices_id;
		glGenBuffers(1, &indices_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * indices.size(), indices.data());

		VAO = vertex_array_object_id;
	}

	RenderObject(Material material, vector<GLfloat>& v_pos, vector<GLfloat>& v_normals, vector<GLuint>& indices, Transform tr) :
		material(material),
		VAO(0),
		n_index(indices.size()),
		transform(tr),
		bbox_min(vec3(0.0)),
		bbox_max(vec3(0.0)) {


		GLuint vertex_array_object_id;
		glGenVertexArrays(1, &vertex_array_object_id);
		glBindVertexArray(vertex_array_object_id);

		// binding positions

		GLuint VBO_NDC_coefficients_id;
		glGenBuffers(1, &VBO_NDC_coefficients_id);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * v_pos.size(), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * v_pos.size(), v_pos.data());


		GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
		GLint  number_of_dimensions = 3; // vec3 em "shader_vertex.glsl"
		glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(location);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// binding normals

		GLuint VBO_normals;
		glGenBuffers(1, &VBO_normals);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * v_normals.size(), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * v_normals.size(), v_normals.data());


		location = 1; // "(location = 0)" em "shader_vertex.glsl"
		number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
		glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(location);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// bind indices

		GLuint indices_id;
		glGenBuffers(1, &indices_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * indices.size(), indices.data());

		VAO = vertex_array_object_id;
	}

	RenderObject(Material material, vector<GLfloat>& v_pos, vector<GLfloat>& v_normals, vector<GLuint>& indices, vector<GLfloat>& v_uvs, Transform tr) :
		material(material),
		VAO(0),
		n_index(indices.size()),
		transform(tr),
		bbox_min(vec3(0.0)),
		bbox_max(vec3(0.0)) {


		GLuint vertex_array_object_id;
		glGenVertexArrays(1, &vertex_array_object_id);
		glBindVertexArray(vertex_array_object_id);

		// binding positions

		GLuint VBO_NDC_coefficients_id;
		glGenBuffers(1, &VBO_NDC_coefficients_id);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * v_pos.size(), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * v_pos.size(), v_pos.data());


		GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
		GLint  number_of_dimensions = 3; // vec3 em "shader_vertex.glsl"
		glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(location);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// binding normals

		GLuint VBO_normals;
		glGenBuffers(1, &VBO_normals);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * v_normals.size(), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * v_normals.size(), v_normals.data());


		location = 1; // "(location = 0)" em "shader_vertex.glsl"
		number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
		glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(location);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// binding uvs

		GLuint VBO_uvs;
		glGenBuffers(1, &VBO_uvs);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_uvs);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * v_uvs.size(), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * v_uvs.size(), v_uvs.data());


		location = 2; // "(location = 0)" em "shader_vertex.glsl"
		number_of_dimensions = 2; // vec4 em "shader_vertex.glsl"
		glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(location);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// bind indices

		GLuint indices_id;
		glGenBuffers(1, &indices_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * indices.size(), indices.data());

		VAO = vertex_array_object_id;
	}
};

class Renderer
{
	static RenderObject grid;
	static RenderObject gizmo;
	static RenderObject ground;
	static int SHADOW_WIDTH;
	static int SHADOW_HEIGHT;
	static GLuint shadowMap_FBO;
	static Shader simpleDepthShader;
	static Shader gizmosShader;
	static Shader groundShader;

public:
	static RenderObject skybox;
	static GLuint g_NumLoadedTextures;
	static GLuint shadowMap;
	static SunLight sun;
	static bool drawGizmos;
	static queue<RenderObject*> renderQ;

	// returns a VAO for the specified vertex array and indice array;
	static GLuint BuildTrianglesVAO(GLfloat* vertex_position, GLfloat* vertex_normals, GLuint* face_indexes, unsigned int n_vert, unsigned int n_index);

	// obsolete
	static void newFrame(GLint width, GLint height);

	static void drawFrame(Camera& currentCamera, int width, int height);

	static void generateGrid();

	static void generateGizmo(Transform transform);

	static void generateGround(Transform transform);

	static void generateShadowMap(int width, int height);

	static void drawGrid(const Camera& camera);

	static void drawGizmo(const Camera& camera, Transform tr);
	
	static void drawLine(const Camera& camera, vec3 dir, vec3 pos, float length, vec4 color = vec4(1.0,0.0,0.0,1.0));
	
	static void drawLine(const Camera& camera, vec3 posa, vec3 posb, vec4 color = vec4(1.0,0.0,0.0,1.0));

	static void drawAABB(const Camera& camera, vec3 bb_min, vec3 bb_max, vec4 color = vec4(1.0,0.0,0.0,1.0));

	static void drawGround(const Camera& camera, Transform tr);

	static void drawSkybox(const Camera& camera);

	static void RenderTriangles(RenderObject& ro, const Camera& camera, bool drawLines);

	static void LoadTextureImage(const char* filename);
};

