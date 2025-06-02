
#include "Block.hpp"
#include "Entities/Entity.hpp"
#include "Player.hpp"
#include "RigidBody.hpp"
#include "Shape.hpp"
#include "Systems/PhysicSystem.hpp"
#include "Systems/RenderSystem.hpp"
#include "Systems/PlayerSystem.hpp"
#include "raylib.h"
#include <numeric>

Scene::Scene()
{
	// Entity ground = this->createEntity();
	// Vector3 dimension = {1000, 1, 1000};
	// OBB obb = {};
	// obb.halfSize = Vector3Scale(dimension, 0.5);
	// ground.addComponent<RigidBodyComponent>(0, (Vector3){0,-0.5,0}, (Vector3){}, (Vector3){},(Matrix){}, obb);
	// Mesh mesh = GenMeshCube(dimension.x, dimension.y, dimension.z);
	// Model model = LoadModelFromMesh(mesh);
	// ground.addComponent<BoxComponent>(model);
	createPlayer(*this, true);
	for (int i = 0; i < 1500; i++)
		setupBlock(*this);
}

Scene::~Scene(void)
{
	auto view = m_registry.view<Model>();
	for (auto entity : view)
	{
		Model &model = m_registry.get<Model>(entity);
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

	playerSystem(m_registry);

	BeginDrawing();
	ClearBackground(LIGHTGRAY);

	renderBlocks(m_registry);

	// BeginMode3D(cam);

	stepPhysicSimulation(m_registry);

	// EndMode3D();

	EndDrawing();
}
