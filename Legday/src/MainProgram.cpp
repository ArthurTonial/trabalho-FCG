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

	Renderer::sun.color = vec4(1.0f);

	// ------------------------------ TEST --------------------------------------------------//
	SpiderTank* myTank = new SpiderTank(0, 6);
	myTank->controlCamera = true;

	Scene::objects.push_back(myTank);

	((PlayerController*)(myTank->getComponents()[1]))->is_pressed = MainWindow::is_pressed;
	((PlayerController*)(myTank->getComponents()[1]))->speed = 5.0f;
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

	while (MainWindow::is_open()) {

		MainWindow::handle_input(MainWindow::window, Scene::mainCamera->MovementSpeed);

		/*if (state >> 3 & 1) {
			printf("Tamo\n");
		}*/

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

