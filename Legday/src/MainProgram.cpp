#include "MainProgram.h"
#define QNT 8

using namespace std;
using namespace glm;

float MainProgram::deltaTime = 0.0;
float MainProgram::lastTime = 0.0;
unsigned int MainProgram::state = 0;
// set up scene

LookAtCamera thirdPersonCamera(vec3(0.0, 20.0, 20.0), vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f);
LookAtCamera FixedCamera(vec3(0.0, 20.0, 20.0), vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f);

Camera* Scene::mainCamera = (Camera*)&thirdPersonCamera;
vector<BaseObject*> Scene::objects;

void updateCameras() {
	Scene::mainCamera->ScreenRatio = ((float)MainWindow::SCR_WIDTH / MainWindow::SCR_HEIGHT);
	((LookAtCamera*)Scene::mainCamera)->target = Scene::objects[0]->transform.position;
	Scene::mainCamera->updateCameraVectors();
}

void MainProgram::init() {
	MainWindow::initUI();

	Renderer::generateGrid();
	Renderer::generateGizmo(Transform(vec3(0.0, 0.0, 0.0), quat(1.0, vec3(0.0, 0.0, 0.0)), vec3(3.0, 3.0, 3.0)));
	Renderer::generateGround(Transform(vec3(0.0, 0.0, 0.0), quat(1.0, vec3(0.0, 0.0, 0.0)), vec3(100.0, 100.0, 100.0)));
	Renderer::generateShadowMap(1024, 1024);

	MainWindow::viewport_tex = Renderer::shadowMap;
	
	Renderer::LoadTextureImage("textures/legTipColor.png");
	Renderer::LoadTextureImage("textures/legTipRME.png");
	Renderer::LoadTextureImage("textures/sphere.png");

	Renderer::sun.color = vec4(1.0f);

	// ------------------------------ TEST --------------------------------------------------//
	SpiderTank* myTank = new SpiderTank(0, 6);
	myTank->controlCamera = true;

	Scene::objects.push_back(myTank);

	((PlayerController*)(myTank->getComponents()[1]))->is_pressed = MainWindow::is_pressed;
	((PlayerController*)(myTank->getComponents()[1]))->speed = 10.0f;
	((PlayerController*)(myTank->getComponents()[1]))->acc = ((PlayerController*)(myTank->getComponents()[1]))->speed * 4;
	((PlayerController*)(myTank->getComponents()[1]))->damp = 3.0f;

	myTank->transform.position += vec3(0.0f, 1.5f, 0.0f);
	myTank->build();
	myTank->transform.lookAt(vec3(1.0, 0.0, 1.0));

	// ------------------------------ TEST --------------------------------------------------//
	
	initObjects();
}

void MainProgram::run() {

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	SpiderTank* st = ((SpiderTank*)Scene::objects[0]);
	Bezier bezier;
	float t = 0, dir = 1;

	while (MainWindow::is_open()) {

		MainWindow::handle_input(MainWindow::window, Scene::mainCamera->MovementSpeed);

		if (deltaTime > 0.0f) {
			deltaTime = glfwGetTime() - lastTime;
			lastTime = glfwGetTime();
		}
		else {
			lastTime = glfwGetTime();
			deltaTime = 1.0f;
		}

		Material::time += deltaTime;

		if ((state >> 0) & 1) {
			state ^= (1 << 0);
			st->resetShader();
		}

		if ((state >> 5) & 1) {

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

		Renderer::sun.debug1 = MainWindow::paramsf[0];
		Renderer::sun.debug2 = MainWindow::paramsf[1];

		Renderer::sun.target = Scene::objects[0]->transform.position;
		Renderer::sun.updateVectors();

		updateCameras();

		updateObjects();

		Renderer::drawFrame(*Scene::mainCamera, MainWindow::SCR_WIDTH, MainWindow::SCR_HEIGHT);

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

