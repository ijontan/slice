#include "Scene.hpp"
#include "raylib.h"
#include "rcamera.h"

#define GLSL_VERSION 330
#define MAX_COLUMNS 20

int main(void)
{
	int screenWidth = 1920;
	int screenHeight = 1080;

	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(screenWidth, screenHeight, "Slicer");

	DisableCursor();
	SetTargetFPS(60);

	Scene *scene =new Scene();

	while (!WindowShouldClose())
	{
		scene->render();
	}

	delete scene;
	CloseWindow();
	return 0;
}
