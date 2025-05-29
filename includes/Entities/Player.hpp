#pragma once
#include "Camera.hpp"
#include "Components/Orientation.hpp"
#include "Components/Position.hpp"
#include "Entity.hpp"
#include "Scene.hpp"

struct PlayerEntity
{
	Entity entity;

	PositionComponent *position;
	OrientationComponent *orientation;
	CameraComponent *mainCamera;

	PlayerEntity(Scene *scene);
};

