#include "Entities/Player.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "raymath.h"

PlayerEntity::PlayerEntity(Scene *scene)
{
	entity = scene->createEntity();
	position = &entity.addComponent<PositionComponent>((Vector3){0.0f, 0.0f, 0.0f});
	orientation = &entity.addComponent<OrientationComponent>(QuaternionIdentity());
	Camera camera = {};
	camera.position = (Vector3){0.0f, 2.0f, 4.0f};
	camera.target = (Vector3){0.0f, 2.0f, 0.0f};
	camera.up = (Vector3){0.0f, 1.0f, 0.0f};
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	mainCamera = &entity.addComponent<CameraComponent>(camera);
}
