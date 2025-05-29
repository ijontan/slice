#include "Scene.hpp"
#include "Block.hpp"
#include "Entities/Entity.hpp"
#include "Player.hpp"
#include "Systems/PhysicSystem.hpp"
#include "Systems/RenderSystem.hpp"
#include "raylib.h"
#include <numeric>

Scene::Scene()
{
	cam = {};
	cam.position = (Vector3){0.0f, 2.0f, 4.0f};
	cam.target = (Vector3){0.0f, 2.0f, 0.0f};
	cam.up = (Vector3){0.0f, 1.0f, 0.0f};
	cam.fovy = 60.0f;
	cam.projection = CAMERA_PERSPECTIVE;

	for (int i = 0; i < 300; i++)
		setupBlock(*this);
}

Scene::~Scene(void)
{
	auto view = m_registry.view<Model>();
	for (auto entity : view)
	{
		Model model = m_registry.get<Model>(entity);
		UnloadMesh(model.meshes[0]);
		UnloadModel(model);
	}
}

Entity Scene::createEntity()
{
	return Entity(m_registry.create(), this);
}
void Scene::render()
{
	if (IsKeyPressed(KEY_E))
	{
		cam_lock = !cam_lock;
		if (cam_lock)
			EnableCursor();
		else
			DisableCursor();
	}

	if (!cam_lock)
		UpdateCamera(&cam, CAMERA_THIRD_PERSON); // Update camera
	else
	{
		Vector3 move = (Vector3){
			(IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) * 0.1f - // Move forward-backward
				(IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * 0.1f,
			(IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) * 0.1f - // Move right-left
				(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * 0.1f,
			0.0f // Move up-down
		};
		UpdateCameraPro(&cam, move, (Vector3){}, 0); // Move to target (zoom)
	}
	float deltaTime = GetFrameTime();
	runTime += deltaTime;

	stepPhysicSimulation(m_registry);

	BeginDrawing();
	ClearBackground(LIGHTGRAY);

	renderBlocks(m_registry, cam);

	BeginMode3D(cam);

	DrawPlane((Vector3){0.0f, 0.0f, 0.0f}, (Vector2){1000.0f, 1000.0f}, DARKGRAY); // Draw ground
	DrawCube(cam.target, 0.5f, 0.5f, 0.5f, PURPLE);
	DrawCubeWires(cam.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);

	EndMode3D();

	EndDrawing();
}
