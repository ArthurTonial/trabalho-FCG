#include "MainProgram.h"
#define QNT 8

using namespace std;
using namespace glm;

float MainProgram::deltaTime = 0.0;
float MainProgram::lastTime = 0.0;
unsigned int MainProgram::state = 0;
// set up scene

LookAtCamera thirdPersonCamera(vec3(0.0, 20.0, 20.0), vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f);
FreeCamera GodCamera(vec3(0.0, 30.0, -30.0), vec3(0.0f, 1.0f, 0.0f), 90.0f, -20.0f);

Camera* Scene::mainCamera = (Camera*)&thirdPersonCamera;
vector<BaseObject*> Scene::objects;

void updateCameras() {
	Scene::mainCamera->ScreenRatio = ((float)MainWindow::SCR_WIDTH / MainWindow::SCR_HEIGHT);
	if (((MainProgram::state >> FREELOOK) & 1)) ((LookAtCamera*)Scene::mainCamera)->target = Scene::objects[0]->transform.position;
	Scene::mainCamera->updateCameraVectors();
}

void generateSkybox() {
	Shader* sh = new Shader("shaders/pbr.vs", "shaders/skybox.fs");
	Mesh* objectMesh = new Mesh("meshes/sphere.obj");

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

	Renderer::skybox = RenderObject(Material(1.0, 0.0, 0.0, vec4(1.0), vec4(1.0), sh), VAO, (int)objectMesh->indices.size(), *new Transform());
}

void MainProgram::init() {
	MainWindow::initUI();

	Renderer::generateGrid();
	Renderer::generateGizmo(Transform(vec3(0.0, 0.0, 0.0), quat(1.0, vec3(0.0, 0.0, 0.0)), vec3(3.0, 3.0, 3.0)));
	Renderer::generateGround(Transform(vec3(0.0, 0.0, 0.0), quat(1.0, vec3(0.0, 0.0, 0.0)), vec3(100.0, 100.0, 100.0)));
	generateSkybox();
	Renderer::generateShadowMap(1024, 1024);

	MainWindow::viewport_tex = Renderer::shadowMap;
	
	Renderer::LoadTextureImage("textures/legTipColor.png");
	Renderer::LoadTextureImage("textures/legTipRME.png");
	Renderer::LoadTextureImage("textures/sphere.png");
	Renderer::LoadTextureImage("textures/wood_D.jpg");
	Renderer::LoadTextureImage("textures/wood_RME.jpg");
	Renderer::LoadTextureImage("textures/skybox.png");

	Renderer::sun.color = vec4(1.0f);

	// ------------------------------ TEST --------------------------------------------------//
	SpiderTank* myTank = new SpiderTank(0, 6);
	myTank->controlCamera = true;

	Scene::objects.push_back(myTank);

	myTank->pc->is_pressed = MainWindow::is_pressed;
	myTank->pc->speed = 10.0f;
	myTank->pc->acc = myTank->pc->speed * 4;
	myTank->pc->damp = 3.0f;

	myTank->transform.position = vec3(0.0, 1.5, 0.0);
	myTank->build(MainWindow::paramsi[0]);
	myTank->transform.lookAt(vec3(0.0, 0.0, 1.0));

	CubeObject* walkable = new CubeObject(1);
	walkable->transform.scale = vec3(200.0, 200.0, 200.0);
	//walkable->transform.lookAt(vec3(1.0,1.0,0.0));
	walkable->transform.position += vec3(0.0, -10.0, 50.0);
	walkable->objectCollision = CollisionObject(walkable->objectMesh, walkable->transform);
	Scene::objects.push_back(walkable);

	// ------------------------------ TEST --------------------------------------------------//
	
	initObjects();
}

void MainProgram::run() {

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	Mesh easteregg("meshes/cow.obj");
	SpiderTank* st = ((SpiderTank*)Scene::objects[0]);
	CubeObject* obstacle = ((CubeObject*)Scene::objects[1]);
	Bezier bezier;
	float t = 0, dir = 1;
	float tim = 0.0f;

	while (MainWindow::is_open()) {

		MainWindow::handle_input(MainWindow::window, Scene::mainCamera->MovementSpeed);

		Renderer::drawGizmos = MainWindow::paramsb[0];
		st->debugMode = MainWindow::paramsb[0];
		obstacle->debugMode = MainWindow::paramsb[0];

		Renderer::drawGizmos = MainWindow::paramsb[0];

		if (deltaTime > 0.0f) {
			deltaTime = glfwGetTime() - lastTime;
			lastTime = glfwGetTime();
		}
		else {
			lastTime = glfwGetTime();
			deltaTime = 1.0f;
		}

		Material::time += deltaTime;

		if (!((MainProgram::state >> 3) & 1)) {
			Scene::mainCamera = (Camera*)&GodCamera;
		}
		else {
			Scene::mainCamera = (Camera*)&thirdPersonCamera;
		}

		if ((state >> COMPSHADERS) & 1) {
			state ^= (1 << COMPSHADERS);
			st->resetShader();
		}

		if ((state >> MOO) & 1) {
			state ^= (1 << MOO);
			printf("ops\n");
			//st->build(MainWindow::paramsi[0]);
			st->mr->setMesh(&easteregg);
			st->mr->componentTransform = Transform(vec3(0.0), quat(1.0, vec3(0.0)), vec3(2.0, 2.0, 2.0));
			st->mr->componentTransform.lookAt(vec3(1.0, 0.0, 0.0));
			st->mr->initComponent();
		}

		if ((state >> MOVE_BEZIER) & 1) {

			if (st->controlCamera) {
				float size = MainWindow::paramsf[3];

				vec3 pos1 = st->transform.position;
				vec3 pos2 = pos1 + size * st->transform.getFront() + 2*size * st->transform.getRight();
				vec3 pos3 = pos1 + size * st->transform.getFront() - 2*size * st->transform.getRight();;
				vec3 pos4 = pos1 + 3*size * st->transform.getFront();

				bezier = Bezier(pos1, pos2, pos3, pos4);
				t = 0, dir = 1;
			}
			float speed = MainWindow::paramsf[2];

			st->controlCamera = false;
			st->transform.position = bezier.getPosition(t);
			st->wantDir = dir * bezier.getTangent(t);

			t += speed * dir * deltaTime;


			if (t > 1.0f) t = 1.0f, dir *= -1;
			if (t < 0.0f) t = 0.0f, dir *= -1;
		}
		else {
			st->controlCamera = true;
		}

		st->t_dist = MainWindow::paramsf[5];
		st->follow_dist = MainWindow::paramsf[6];

		Renderer::sun.debug1 = MainWindow::paramsf[0];
		Renderer::sun.debug2 = MainWindow::paramsf[1];
		Renderer::sun.target = Scene::objects[0]->transform.position;
		Renderer::sun.updateVectors();

		Renderer::drawFrame(*Scene::mainCamera, MainWindow::SCR_WIDTH, MainWindow::SCR_HEIGHT);

		updateCameras();

		updateObjects();

		MainWindow::RenderUI();

		glfwSwapBuffers(MainWindow::window);
	}
	MainWindow::cleanupUI();
}

void MainProgram::initObjects() {
	for (BaseObject* obj : Scene::objects) {
		obj->init();
	}
	// run init for components
	for (BaseObject* obj : Scene::objects) {
		obj->initComponents();
	}
}

void MainProgram::updateObjects() {
	for (BaseObject* obj : Scene::objects) {
		obj->run();
	}
	for (BaseObject* obj : Scene::objects) {
		obj->runComponents();
	}
}

