
#include "Block.hpp"
#include "Enemy.hpp"
#include "EnemySystem.hpp"
#include "Entities/Entity.hpp"
#include "FixedJoint.hpp"
#include "Player.hpp"
#include "RigidBody.hpp"
#include "Shape.hpp"
#include "Systems/PhysicSystem.hpp"
#include "Systems/PlayerSystem.hpp"
#include "Systems/RenderSystem.hpp"
#include "raylib.h"
#include "raymath.h"
#include <numeric>

Scene::Scene()
{

	createPlayer(*this, true);
	createEnemy(*this);
	createEnemy(*this);
	createEnemy(*this);
	createEnemy(*this);

	for (int i = 0; i < 300; i++)
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
	enemyRegenBodySystem(m_registry, *this);

	BeginDrawing();
	ClearBackground(LIGHTGRAY);

	renderBlocks(m_registry);

	stepPhysicSimulation(m_registry);

	EndDrawing();
}
