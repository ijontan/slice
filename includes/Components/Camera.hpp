#pragma once
#include "raylib.h"

struct CameraComponent
{
	Camera camera;

	CameraComponent() = default;
	~CameraComponent() = default;
	CameraComponent(Camera &camera) : camera(camera)
	{
	}
};
