#include "Scene.hpp"
#include "raylib.h"
#include "rcamera.h"

#define GLSL_VERSION 330
#define MAX_COLUMNS 20

int main(void)
{
	int screenWidth = 1920;
	int screenHeight = 1080;
	InitWindow(screenWidth, screenHeight, "Slicer");

	DisableCursor();
	SetTargetFPS(60);

	Scene scene = Scene();

	while (!WindowShouldClose())
	{
		scene.render();
	}

	CloseWindow();
	return 0;
}
