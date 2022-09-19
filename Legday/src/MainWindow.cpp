#include "MainWindow.h"

int MainWindow::SCR_WIDTH = 1500;
int MainWindow::SCR_HEIGHT = 1000;
bool MainWindow::is_open_bool;
float MainWindow::paramsf[8] = {0.0, 0.0, 20, 0, 2.66, 2.0, 2.0, 5.5};
int MainWindow::paramsi[8] = {3, 0, 0, 0, 0, 0, 0, 0};
bool MainWindow::is_pressed[6] = { false, false ,false ,false ,false ,false };
GLFWwindow* MainWindow::window;
GLuint MainWindow::viewport_tex;


int MainWindow::initUI() {
	is_open_bool = 1;
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) return 1;

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Program", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;    // Enable mouse Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
	io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 20);
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	//	ImGuiDockNodeFlags_PassthruCentralNode
}

void MainWindow::RenderUI() {
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	glfwPollEvents();

	// start UI frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// UI setup
	
	// drawOptions MUST come before drawViewport
	drawOptions();
	drawViewport();
	
	// Renders the ImGUI elements
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}

void MainWindow::cleanupUI() {
	// Deletes all ImGUI instances
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

int MainWindow::is_open() {
	return (!(glfwWindowShouldClose(window)) and is_open_bool);
}

void MainWindow::handle_input(GLFWwindow* window, float _speed) {

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	if (ImGui::IsKeyDown('W')) {
		is_pressed[2] = true;
	}
	else is_pressed[2] = false;

	if (ImGui::IsKeyDown('A')) {
		is_pressed[3] = true;

	}
	else is_pressed[3] = false;

	if (ImGui::IsKeyDown('S')) {
		is_pressed[4] = true;

	}
	else is_pressed[4] = false;

	if (ImGui::IsKeyDown('D')) {
		is_pressed[5] = true;

	}
	else is_pressed[5] = false;

	if (ImGui::IsKeyDown(ImGuiKey_Space)) {
	}
	if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
		MainProgram::state ^= (3 << 0);
		if ((MainProgram::state & (3 << 0)) == (3 << 0)) {
			glfwSetInputMode(MainWindow::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else {
			glfwSetInputMode(MainWindow::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	// imgui mouse
	if ((MainProgram::state & (3 << 0)) == (3 << 0)) {
	}

}

void MainWindow::drawViewport() {

	ImGuiWindowFlags window_flags = 0;

	//window_flags |= ImGuiWindowFlags_NoTitleBar;
	//window_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoResize;

	ImGui::SetNextWindowSize(ImVec2(1024.0f, 1024.0f));
	ImGui::Begin("ShadowMap", NULL, window_flags);


	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddImage((void*)viewport_tex,
		pos,
		ImVec2(pos.x + ImGui::GetWindowSize().x - 10, pos.y + ImGui::GetWindowSize().y),
		ImVec2(0, 1),
		ImVec2(1, 0));

	ImGui::End();
}

void MainWindow::drawOptions() {
	using namespace ImGui;

	Begin("Options");


	if (Button("Switch Shaders", ImVec2(150.0f, 50.0f))) {
		MainProgram::state ^= (1 << 0);
	}
	if (Button("Reset", ImVec2(150.0f, 50.0f))) {
		MainProgram::state |= (1 << 2);
	}

	SliderFloat("camera_X", &paramsf[0], -10.0, 10.0, "%.5f");
	SliderFloat("camera_Y", &paramsf[1], -10.0, 10.0, "%.5f");
	SliderFloat("camera_Z", &paramsf[2], -10.0, 10.0, "%.5f");
	SliderFloat("camera_YAW", &paramsf[3], -90.0, 90.0, "%.5f");
	SliderFloat("camera_PITCH", &paramsf[4], -90.0, 90.0, "%.5f");
	SliderFloat("stateX", &paramsf[5], -10.0f, 10.0f, "%.5f");
	SliderFloat("stateY", &paramsf[6], -10.0f, 10.0f, "%.5f");
	SliderFloat("stateZ", &paramsf[7], -10.0f, 10.0f, "%.5f");

	ImGui::Text("%d", paramsi[0]);
	SameLine();
	BeginGroup();
	if (Button("+", ImVec2(30.0f, 30.0f))) paramsi[0]++;
	if (Button("-", ImVec2(30.0f, 30.0f))) paramsi[0]--;
	EndGroup();


	End();
}

void MainWindow::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{    // make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.

	MainWindow::SCR_HEIGHT = height;
	MainWindow::SCR_WIDTH = width;

	return;
}