#include "Renderer.h"

Shader Renderer::ivShader;
RenderObject Renderer::grid;
RenderObject Renderer::gizmo;
RenderObject Renderer::ground;
int Renderer::SHADOW_WIDTH = 1024;
int Renderer::SHADOW_HEIGHT = 1024;
SunLight Renderer::sun;
GLuint Renderer::shadowMap_FBO = 0;
GLuint Renderer::shadowMap;
Shader Renderer::simpleDepthShader;
queue<RenderObject> Renderer::renderQ;

void Renderer::generateGrid() {
	ivShader = Shader("IV.vs", "IV.fs");

	GLfloat v_grid[50 * 50][3];
	GLuint* i_grid;

	float z = -50.0f;
	for (int i = 0; i < 50; i++) {
		float x = -50.0f;
		for (int j = 0; j < 50; j++) {
			v_grid[(50 * i + j)][0] = x;
			v_grid[(50 * i + j)][1] = 0.2f;
			v_grid[(50 * i + j)][2] = z;
			x += 2.0f;
		}
		z += 2.0f;
	}

	vector<GLuint> temp;

	int k = 0;
	for (int i = 0; i < 50; i++) {
		for (int j = 0; j < 50; j++) {
			// 50 * 50 * 4 - (50 * 2 + 50 * 2)
			if (j < 49) {
				temp.push_back(k);
				temp.push_back(k+1);
			}

			if (i < 49) {
				temp.push_back(k);
				temp.push_back(k + 50);
			}
			k++;
		}
	}

	int n_idx = temp.size();
	i_grid = (GLuint * )malloc(sizeof(GLuint) * n_idx);

	for (int i = 0; i < n_idx; i++) i_grid[i] = temp[i];

	GLuint vertex_array_object_id;
	glGenVertexArrays(1, &vertex_array_object_id);
	glBindVertexArray(vertex_array_object_id);

	// binding positions

	GLuint VBO_NDC_coefficients_id;
	glGenBuffers(1, &VBO_NDC_coefficients_id);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 50*50, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 3 * 50*50, v_grid);


	GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
	GLint  number_of_dimensions = 3; // vec4 em "shader_vertex.glsl"
	glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(location);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// bind indices

	GLuint indices_id;
	glGenBuffers(1, &indices_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * n_idx, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * n_idx, i_grid);

	Shader sh = Shader("shaders/gizmos.vs", "shaders/gizmos.fs");

	grid = RenderObject(new Material(1.0f, 0.0f, 0.0f, vec4(0.3f, 0.3f, 0.3f, 1.0f), vec4(0.0f), sh), vertex_array_object_id, n_idx, Transform());
}

void Renderer::generateGizmo(Transform transform) {

	vector<GLfloat> v_gizmos = {
		0.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,0.0f,1.0f
	};
	vector<GLuint> i_gizmos = {
		0,1,
		0,2,
		0,3
	};

	Shader sh = Shader("shaders/gizmos.vs", "shaders/gizmos.fs");

	gizmo = RenderObject(new Material(1.0f, 0.0f, 0.0f, vec4(0.3f, 0.3f, 0.3f, 1.0f), vec4(0.0f), sh), v_gizmos, i_gizmos, transform);
}

void Renderer::generateGround(Transform transform) {

	vector<GLfloat> v_ground = {
		-1.0f,0.0f,-1.0f,
		-1.0f,0.0f,1.0f,
		1.0f,0.0f,1.0f,
		1.0f,0.0f,-1.0f
	};

	vector<GLfloat> v_normals = {
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f
	};

	vector<GLuint> i_ground = {
		0, 1, 3,
		1, 2, 3
	};

	Shader sh = Shader("shaders/ground.vs", "shaders/ground.fs");

	ground = RenderObject(new Material(1.0f, 1.0f, 0.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), sh), v_ground, v_normals, i_ground, transform);
}

void Renderer::generateShadowMap(int width, int height) {

	SHADOW_HEIGHT = height;
	SHADOW_WIDTH = width;

	simpleDepthShader = Shader("shaders/simpleDepth.vs", "shaders/simpleDepth.fs");

	if (shadowMap_FBO == 0) {
		glGenFramebuffers(1, &shadowMap_FBO);
	}

	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMap_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::drawGrid(const Camera& camera) {
	grid.material->setShaderOptions(camera, grid.transform);

	glBindVertexArray(grid.VAO);
	glLineWidth(5.0f);
	glDrawElements(GL_LINES, grid.n_index, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Renderer::drawGizmo(const Camera& camera, Transform tr = Transform()) {
	gizmo.material->sh.use();
	gizmo.material->sh.setMatrix4("mode_m", Transform(
		tr.position + (tr.rotation * (tr.scale * gizmo.transform.position)),
		tr.rotation * gizmo.transform.rotation,
		gizmo.transform.scale * tr.scale).GetModelMatrix());
	gizmo.material->sh.setMatrix4("view_m", camera.GetViewMatrix());
	gizmo.material->sh.setMatrix4("proj_m", camera.GetProjectionMatrix());

	gizmo.material->sh.setFloat("diffuse", 1.0f);
	gizmo.material->sh.setFloat("glossy", 0.0f);
	gizmo.material->sh.setFloat("metallic", 0.0f);
	gizmo.material->sh.setFloat4("glossy_color", vec4(0.0f));

	glBindVertexArray(gizmo.VAO);
	glLineWidth(10.0f);

	gizmo.material->sh.setFloat4("diffuse_color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0));

	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);

	gizmo.material->sh.setFloat4("diffuse_color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0));

	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(2 * sizeof(GLuint)));

	gizmo.material->sh.setFloat4("diffuse_color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0));

	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(4 * sizeof(GLuint)));

	glBindVertexArray(0);
}

void Renderer::drawGround(const Camera& camera, Transform tr = Transform()) {

	ground.material->setShaderOptions(camera, sun, Transform(
		tr.position + (tr.rotation * (tr.scale * ground.transform.position)),
		tr.rotation * ground.transform.rotation,
		ground.transform.scale * tr.scale));

	renderQ.push(ground);
}

GLuint Renderer::BuildTrianglesVAO(GLfloat* vertex_position, GLfloat* vertex_normals, GLuint* face_indexes, unsigned int n_vert, unsigned int n_index)
{

	GLuint vertex_array_object_id;
	glGenVertexArrays(1, &vertex_array_object_id);
	glBindVertexArray(vertex_array_object_id);
	
	// binding positions

	GLuint VBO_NDC_coefficients_id;
	glGenBuffers(1, &VBO_NDC_coefficients_id);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * n_vert, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 3 * n_vert, vertex_position);


	GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
	GLint  number_of_dimensions = 3; // vec4 em "shader_vertex.glsl"
	glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(location);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// binding normals

	GLuint VBO_normals;
	glGenBuffers(1, &VBO_normals);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * n_vert, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 4 * n_vert, vertex_normals);


	location = 1; // "(location = 0)" em "shader_vertex.glsl"
	number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
	glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(location);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// binding indices

	// Criamos um buffer OpenGL para armazenar os índices acima
	GLuint indices_id;
	glGenBuffers(1, &indices_id);

	// "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);

	// Alocamos memória para o buffer.
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * n_index,NULL, GL_STATIC_DRAW);

	// Copiamos os valores do array indices[] para dentro do buffer.
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * n_index, face_indexes);

	// NÃO faça a chamada abaixo! Diferente de um VBO (GL_ARRAY_BUFFER), um
	// array de índices (GL_ELEMENT_ARRAY_BUFFER) não pode ser "desligado",
	// caso contrário o VAO irá perder a informação sobre os índices.
	//
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
	//

	// "Desligamos" o VAO, evitando assim que operações posteriores venham a
	// alterar o mesmo. Isso evita bugs.
	glBindVertexArray(0);

	// Retornamos o ID do VAO. Isso é tudo que será necessário para renderizar
	// os triângulos definidos acima. Veja a chamada glDrawElements() em main().

	

	return vertex_array_object_id;
}

void Renderer::RenderTriangles(RenderObject ro, const Camera& camera, bool drawLines) {
	//std::cout << "n_idx: " << (unsigned int)n_index << std::endl;
	
	ro.material->setShaderOptions(camera, Renderer::sun, ro.transform);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowMap);

	glBindVertexArray(ro.VAO);
	
	glCullFace(GL_BACK);
	glDrawElements(GL_TRIANGLES, ro.n_index, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void Renderer::drawFrame(Camera& currentCamera, int FRAME_WIDTH, int FRAME_HEIGHT) {
	//drawGround(currentCamera);

	renderQ.push(ground);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	if (shadowMap_FBO != 0) {
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMap_FBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		// set light view matrix
		int n_objs = renderQ.size();

		while (n_objs--) {
			RenderObject cur = renderQ.front();

			simpleDepthShader.use();
			simpleDepthShader.setMatrix4("mode_m", cur.transform.GetModelMatrix());
			simpleDepthShader.setMatrix4("view_m", sun.GetViewMatrix());
			simpleDepthShader.setMatrix4("proj_m", sun.GetProjectionMatrix());

			glBindVertexArray(cur.VAO);

			glCullFace(GL_BACK);
			glDrawElements(GL_TRIANGLES, cur.n_index, GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);

			renderQ.pop();
			renderQ.push(cur);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.46, 0.46, 0.46, 1.0);

	drawGrid(currentCamera);
	drawGizmo(currentCamera);
	
	glViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);

	// "Pintamos" todos os pixels do framebuffer com a cor definida acima

	while (!renderQ.empty()) {
		RenderObject cur = renderQ.front();

		//drawGizmo(currentCamera, cur.transform);

		RenderTriangles(cur, currentCamera, false);

		renderQ.pop(); 
	}
}


// obsolete
void Renderer::newFrame(GLint width, GLint height) {

	
}
