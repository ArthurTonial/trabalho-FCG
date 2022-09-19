#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "MainProgram.h"

class MainWindow
{
	static bool is_open_bool;

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

	static void glfw_error_callback(int error, const char* description)
	{
		fprintf(stderr, "Glfw Error %d: %s\n", error, description);
	}

public:

	static float paramsf[8];
	static int paramsi[8];
	static bool is_pressed[6];
	static GLFWwindow* window;
	static GLuint viewport_tex;
	static int SCR_WIDTH;
	static int SCR_HEIGHT;
	static int initUI();
	static void RenderUI();
	static void cleanupUI();
	static int is_open();
	static void handle_input(GLFWwindow* window, float _speed);
	static void drawViewport();
	static void drawOptions();
};

