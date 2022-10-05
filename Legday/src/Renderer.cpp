#include "Renderer.h"

float Material::time;
RenderObject Renderer::grid;
RenderObject Renderer::gizmo;
RenderObject Renderer::ground;
RenderObject Renderer::skybox;
int Renderer::SHADOW_WIDTH = 1024;
int Renderer::SHADOW_HEIGHT = 1024;
SunLight Renderer::sun;
GLuint Renderer::shadowMap_FBO = 0;
GLuint Renderer::g_NumLoadedTextures = 0;
GLuint Renderer::shadowMap;
Shader Renderer::simpleDepthShader;
Shader Renderer::gizmosShader;
Shader Renderer::groundShader;
bool Renderer::drawGizmos = false;
queue<RenderObject*> Renderer::renderQ;

void Renderer::generateGrid() {

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

	gizmosShader = Shader("shaders/gizmos.vs", "shaders/gizmos.fs");

	grid = RenderObject(Material(1.0f, 0.0f, 0.0f, vec4(0.3f, 0.3f, 0.3f, 1.0f), vec4(0.0f), &gizmosShader), vertex_array_object_id, n_idx, Transform());
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

	gizmosShader = Shader("shaders/gizmos.vs", "shaders/gizmos.fs");

	gizmo = RenderObject(Material(1.0f, 0.0f, 0.0f, vec4(0.3f, 0.3f, 0.3f, 1.0f), vec4(0.0f), &gizmosShader), v_gizmos, i_gizmos, transform);
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

	vector<GLfloat> v_uvs = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
	};

	vector<GLuint> i_ground = {
		0, 1, 3,
		1, 2, 3
	};

	groundShader = Shader("shaders/pbr.vs", "shaders/ground.fs");

	ground = RenderObject(Material(1.0f, 1.0f, 0.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), &groundShader), v_ground, v_normals, i_ground, v_uvs, transform);
}

void Renderer::generateShadowMap(int width, int height) {

	SHADOW_HEIGHT = height;
	SHADOW_WIDTH = width;

	simpleDepthShader = Shader("shaders/simpleDepth.vs", "shaders/simpleDepth.fs");
	/*
	// Agora criamos objetos na GPU com OpenGL para armazenar a textura
	GLuint sampler_id;
	glGenTextures(1, &shadowMap);
	glGenSamplers(1, &sampler_id);

	// Veja slides 95-96 do documento Aula_20_Mapeamento_de_Texturas.pdf
	glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// Parâmetros de amostragem da textura.
	glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLfloat border_color[4] = { 1.0,1.0,1.0,1.0 };
	glSamplerParameterfv(sampler_id, GL_TEXTURE_BORDER_COLOR, border_color);

	GLuint textureunit = g_NumLoadedTextures;
	glActiveTexture(GL_TEXTURE0 + textureunit);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindSampler(textureunit, sampler_id);

	g_NumLoadedTextures += 1;*/
	
	// generates texture for shadowmap buffer
	glGenTextures(1, &shadowMap);
	// binds shadowmap to slot
	GLuint textureunit = g_NumLoadedTextures;
	glActiveTexture(GL_TEXTURE0 + textureunit);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	g_NumLoadedTextures += 1;

	// sets shadowmap parameters
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat border_color[4] = { 1.0,1.0,1.0,1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);


	// sets framebuffer parameters
	// generate new framebuffer where shadowMap scene will be draw on
	if (shadowMap_FBO == 0) {
		glGenFramebuffers(1, &shadowMap_FBO);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMap_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Renderer::drawGrid(const Camera& camera) {
	grid.material.setShaderOptions(camera, grid.transform);
	grid.material.sh->use();

	glBindVertexArray(grid.VAO);
	glLineWidth(5.0f);
	glDrawElements(GL_LINES, grid.n_index, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Renderer::drawGizmo(const Camera& camera, Transform tr = Transform()) {
	tr.scale *= 2.0f;

	gizmo.material.sh->use();
	gizmo.material.sh->setMatrix4("mode_m", tr.GetModelMatrix());
	gizmo.material.sh->setMatrix4("view_m", camera.GetViewMatrix());
	gizmo.material.sh->setMatrix4("proj_m", camera.GetProjectionMatrix());

	gizmo.material.sh->setFloat("diffuse", 1.0f);
	gizmo.material.sh->setFloat("glossy", 0.0f);
	gizmo.material.sh->setFloat("metallic", 0.0f);
	gizmo.material.sh->setFloat4("glossy_color", vec4(0.0f));

	glBindVertexArray(gizmo.VAO);
	glLineWidth(10.0f);

	gizmo.material.sh->setFloat4("diffuse_color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0));

	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);

	gizmo.material.sh->setFloat4("diffuse_color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0));

	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(2 * sizeof(GLuint)));

	gizmo.material.sh->setFloat4("diffuse_color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0));

	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(4 * sizeof(GLuint)));

	glBindVertexArray(0);
}

void Renderer::drawLine(const Camera& camera, vec3 posa, vec3 posb, vec4 color) {
	vec3 d = (posb - posa);
	drawLine(camera, normalize(d), posa, length(d), color);
}

void Renderer::drawLine(const Camera& camera, vec3 dir, vec3 pos, float length, vec4 color) {
	Transform tr = Transform(pos, quat(1.0, vec3(0.0, 0.0, 0.0)), vec3(length));
	tr.lookAt(-dir);

	gizmo.material.sh->use();
	gizmo.material.sh->setMatrix4("mode_m", tr.GetModelMatrix());
	gizmo.material.sh->setMatrix4("view_m", camera.GetViewMatrix());
	gizmo.material.sh->setMatrix4("proj_m", camera.GetProjectionMatrix());

	gizmo.material.sh->setFloat("diffuse", 1.0f);
	gizmo.material.sh->setFloat("glossy", 0.0f);
	gizmo.material.sh->setFloat("metallic", 0.0f);
	gizmo.material.sh->setFloat4("glossy_color", vec4(0.0f));

	glBindVertexArray(gizmo.VAO);
	glLineWidth(5.0f);

	gizmo.material.sh->setFloat4("diffuse_color", color);

	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(4 * sizeof(GLuint)));

	glBindVertexArray(0);
}

void Renderer::drawAABB(const Camera& camera, vec3 bb_min, vec3 bb_max, vec4 color) {
	vec3 v[8] = {
		bb_min,
		vec3(bb_min.x, bb_min.y, bb_max.z),
		vec3(bb_max.x, bb_min.y, bb_max.z),
		vec3(bb_max.x, bb_max.y, bb_max.z),
		vec3(bb_max.x, bb_max.y, bb_min.z),
		vec3(bb_min.x, bb_max.y, bb_min.z),
		vec3(bb_min.x, bb_max.y, bb_max.z),
		vec3(bb_max.x, bb_min.y, bb_min.z)
	};

	drawLine(camera, v[0], v[1], color);
	drawLine(camera, v[0], v[5], color);
	drawLine(camera, v[6], v[1], color);
	drawLine(camera, v[5], v[6], color);

	drawLine(camera, v[3], v[2], color);
	drawLine(camera, v[4], v[3], color);
	drawLine(camera, v[7], v[2], color);
	drawLine(camera, v[7], v[4], color);

	drawLine(camera, v[0], v[7], color);
	drawLine(camera, v[1], v[2], color);
	drawLine(camera, v[3], v[6], color);
	drawLine(camera, v[4], v[5], color);
}

void Renderer::drawGround(const Camera& camera, Transform tr = Transform()) {

	ground.material.setShaderOptions(camera, sun, Transform(
		tr.position + (tr.rotation * (tr.scale * ground.transform.position)),
		tr.rotation * ground.transform.rotation,
		ground.transform.scale * tr.scale));

	renderQ.push(&ground);
}

void Renderer::drawSkybox(const Camera& camera) {
	skybox.transform.position = camera.Position;
	skybox.transform.scale = vec3(2.0f);

	skybox.material.sh->use();
	skybox.material.setShaderOptions(camera, Renderer::sun, skybox.transform);

	glBindVertexArray(skybox.VAO);

	glCullFace(GL_BACK);
	glDrawElements(GL_TRIANGLES, skybox.n_index, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

// obsolete
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

void Renderer::RenderTriangles(RenderObject& ro, const Camera& camera, bool drawLines) {
	//std::cout << "n_idx: " << (unsigned int)n_index << std::endl;
	
	ro.material.sh->use();
	ro.material.setShaderOptions(camera, Renderer::sun, ro.transform);

	glBindVertexArray(ro.VAO);
	
	glCullFace(GL_BACK);
	glDrawElements(GL_TRIANGLES, ro.n_index, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void Renderer::drawFrame(Camera& currentCamera, int FRAME_WIDTH, int FRAME_HEIGHT) {
	//drawGround(currentCamera);

	renderQ.push(&ground);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// draws scene from sun's ortographic perspective 
	if (shadowMap_FBO != 0) {
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMap_FBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		// set light view matrix
		int n_objs = renderQ.size();

		while (n_objs--) {
			RenderObject* cur = renderQ.front();

			simpleDepthShader.use();
			simpleDepthShader.setMatrix4("mode_m", cur->transform.GetModelMatrix());
			simpleDepthShader.setMatrix4("view_m", sun.GetViewMatrix());
			simpleDepthShader.setMatrix4("proj_m", sun.GetProjectionMatrix());

			simpleDepthShader.setInt("shadowMap", 0);
			simpleDepthShader.setInt("TextureImage1", 1);
			simpleDepthShader.setInt("TextureImage2", 2);

			glBindVertexArray(cur->VAO);

			glCullFace(GL_BACK);
			glDrawElements(GL_TRIANGLES, cur->n_index, GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);

			renderQ.pop();
			renderQ.push(cur);
		}
	}


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.46, 0.46, 0.46, 1.0);


	glFrontFace(GL_CW);
	drawSkybox(currentCamera);
	glClear(GL_DEPTH_BUFFER_BIT);
	glFrontFace(GL_CCW);

	if (drawGizmos) {
		drawGrid(currentCamera);
		drawGizmo(currentCamera);
	}
	
	glViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT);

	// "Pintamos" todos os pixels do framebuffer com a cor definida acima

	while (!renderQ.empty()) {
		RenderObject* cur = renderQ.front();

		if(renderQ.size() > 1 and drawGizmos) drawGizmo(currentCamera, cur->transform);

		RenderTriangles(*cur, currentCamera, false);

		renderQ.pop(); 
	}
}

// obsolete
void Renderer::newFrame(GLint width, GLint height) {

	
}

// Função que carrega uma imagem para ser utilizada como textura
void Renderer::LoadTextureImage(const char* filename)
{
	printf("Carregando imagem \"%s\"... ", filename);

	// Primeiro fazemos a leitura da imagem do disco
	stbi_set_flip_vertically_on_load(true);
	int width;
	int height;
	int channels;
	unsigned char* data = stbi_load(filename, &width, &height, &channels, 3);

	if (data == NULL)
	{
		fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
		std::exit(EXIT_FAILURE);
	}

	printf("OK (%dx%d).\n", width, height);

	// Agora criamos objetos na GPU com OpenGL para armazenar a textura
	GLuint texture_id;
	GLuint sampler_id;
	glGenTextures(1, &texture_id);
	glGenSamplers(1, &sampler_id);

	// Veja slides 95-96 do documento Aula_20_Mapeamento_de_Texturas.pdf
	glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Parâmetros de amostragem da textura.
	glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Agora enviamos a imagem lida do disco para a GPU
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

	GLuint textureunit = g_NumLoadedTextures;
	glActiveTexture(GL_TEXTURE0 + textureunit);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindSampler(textureunit, sampler_id);

	stbi_image_free(data);

	g_NumLoadedTextures += 1;
}
