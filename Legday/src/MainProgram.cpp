#include "MainProgram.h"
#define QNT 8

using namespace std;
using namespace glm;

unsigned int MainProgram::state = 0;
// set up scene

LookAtCamera thirdPersonCamera(vec3(0.0, 20.0, 20.0), vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f);
LookAtCamera FixedCamera(vec3(0.0, 20.0, 20.0), vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f);

Camera* Scene::mainCamera = (Camera*)&thirdPersonCamera;
vector<BaseObject*> Scene::objects;

void updateCameras() {
	Scene::mainCamera->ScreenRatio = ((float)MainWindow::SCR_WIDTH / MainWindow::SCR_HEIGHT);
	if (Scene::objects.size()) {
		((LookAtCamera*)Scene::mainCamera)->target = Scene::objects[0]->transform.position;
	}
	Scene::mainCamera->updateCameraVectors();
}

void MainProgram::init() {
	MainWindow::initUI();

	Renderer::generateGrid();
	Renderer::generateGizmo(Transform(vec3(0.0, 0.0, 0.0), quat(1.0, vec3(0.0, 0.0, 0.0)), vec3(3.0, 3.0, 3.0)));
	Renderer::generateGround(Transform(vec3(0.0, 0.0, 0.0), quat(1.0, vec3(0.0, 0.0, 0.0)), vec3(100.0, 100.0, 100.0)));
	//glfwSetCursorPosCallback(MainWindow::window, mouse_callback);
	//glfwSetScrollCallback(MainWindow::window, scroll_callback);

	initObjects();
}

void MainProgram::run() {

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	while (MainWindow::is_open()) {

		MainWindow::handle_input(MainWindow::window, 10.0f);

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