
#include "AnimationSystem.hpp"
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
	BlockFactory blockFactory(*this);

	createPlayer(*this, true);
	createEnemy(*this);
	createEnemy(*this);
	createEnemy(*this);
	createEnemy(*this);

	for (int i = 0; i < 300; i++)
	{
		blockFactory.randomize();
		blockFactory.generateBlock();
	}
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

	animateBlockSpawn(m_registry);
	stepPhysicSimulation(m_registry);
	renderBlocks(m_registry);
	EndDrawing();
}
