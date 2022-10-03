#include "MainWindow.h"

int MainWindow::SCR_WIDTH = 1500;
int MainWindow::SCR_HEIGHT = 1000;
bool MainWindow::is_open_bool;
float MainWindow::paramsf[8] = {150.0, 30.0, 0.5, 1.0, 2.5, 2.5, 1.7, 5.5};
int MainWindow::paramsi[8] = {2, 0, 0, 0, 0, 0, 0, 0};
bool MainWindow::is_pressed[6] = { false, false ,false ,false ,false ,false };
GLFWwindow* MainWindow::window;
GLuint MainWindow::viewport_tex;
bool options_hovered;


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
		MainProgram::state ^= (1 << 3);
		if ((MainProgram::state >> 3) & 1) {
			glfwSetInputMode(MainWindow::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else {
			glfwSetInputMode(MainWindow::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	

	if (ImGui::IsKeyPressed('B')) {
		MainProgram::state ^= (1 << 5);
	}

	// imgui mouse
	if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
		Renderer::sun.ProcessMouseMovement(io.MouseDelta.x, io.MouseDelta.y);
	}
	if ((MainProgram::state >> 3) & 1) {
		Scene::mainCamera->ProcessMouseMovement(io.MouseDelta.x, -io.MouseDelta.y, true);
	}
	else {
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left) and !options_hovered) {
			Scene::mainCamera->ProcessMouseMovement(io.MouseDelta.x, -io.MouseDelta.y, true);
		}
	}


}

void MainWindow::drawViewport() {

	ImGuiWindowFlags window_flags = 0;

	//window_flags |= ImGuiWindowFlags_NoTitleBar;
	//window_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoResize;

	ImGui::SetNextWindowSize(ImVec2(512.0f, 512.0f));
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
	if (ImGui::IsWindowHovered() or ImGui::IsAnyItemHovered()) {
		options_hovered = true;
	}
	else {
		options_hovered = false;
	}


	if (Button("Comp Shaders", ImVec2(150.0f, 50.0f))) {
		MainProgram::state ^= (1 << 0);
	}
	if (Button("Moooo", ImVec2(150.0f, 50.0f))) {
		MainProgram::state ^= (1 << 2);
	}

	SliderFloat("Sun nearplane", &paramsf[0], 10.0, 1000.0, "%.2f");
	SliderFloat("Sun fov", &paramsf[1], 5.0, 50.0, "%.5f");
	SliderFloat("Bezier Path Speed", &paramsf[2], 0.0, 1.0, "%.5f");
	SliderFloat("Bezier Path Size", &paramsf[3], 1.0, 10.0, "%.5f");
	//SliderInt("N Segs", &paramsi[0], 2, 6);
	SliderFloat("Leg Spread", &paramsf[5], 0.5f, 10.0f, "%.5f");
	SliderFloat("Follow Dist", &paramsf[6], 0.0f, 5.0f, "%.5f");
	//SliderFloat("stateZ", &paramsf[7], -10.0f, 10.0f, "%.5f");


	End();
}

void MainWindow::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{    // make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.

	MainWindow::SCR_HEIGHT = height;
	MainWindow::SCR_WIDTH = width;

	return;
}