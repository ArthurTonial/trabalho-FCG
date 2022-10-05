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
#include "BaseObject.h"
#include "MeshRenderer.h"
#include "Scene.h"
#include "Collision.h"
#include "Bezier.h"

#include "IKleg.h"
#include "SpiderTank.h"

#define MOVE_BEZIER 5
#define FREELOOK 3
#define COMPSHADERS 0
#define MOO 2

using namespace std;

class MainProgram
{
public:

	static float deltaTime;
	static float lastTime;
	static unsigned int state;
	static void initObjects();
	static void updateObjects();
	static void init();
	static void run();
};

