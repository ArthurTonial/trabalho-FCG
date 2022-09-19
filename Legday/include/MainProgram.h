#pragma once

#include<iostream>
#include<string>
#include<vector>
#include<algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>

#include "MainWindow.h"

using namespace std;

class MainProgram
{
public:

	static unsigned int state;
	static void init();
	static void run();
};

