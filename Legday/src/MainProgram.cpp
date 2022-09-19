#include "MainProgram.h"
#define QNT 8

using namespace std;
using namespace glm;

unsigned int MainProgram::state = 0;
// set up scene

void MainProgram::init() {
	MainWindow::initUI();

	//glfwSetCursorPosCallback(MainWindow::window, mouse_callback);
	//glfwSetScrollCallback(MainWindow::window, scroll_callback);
}

void MainProgram::run() {

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	while (MainWindow::is_open()) {

		MainWindow::handle_input(MainWindow::window, 10.0f);

		MainWindow::RenderUI();
		
		glfwSwapBuffers(MainWindow::window);
	}
	MainWindow::cleanupUI();
}
