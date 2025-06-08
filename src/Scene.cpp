
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
	// Entity a = setupBlock(*this, {2, 1, 0}, {1, 1, 1}, {0, 0, 0}, {0, 0, 0}, CollisionMask::ENEMY,
	// 					  CollisionMask::FREE | CollisionMask::PLAYER);
	// Entity b = setupBlock(*this, {2, 2, 0}, {1, 1, 1}, {0, 0, 0}, {0, 0, 0}, CollisionMask::ENEMY,
	// 					  CollisionMask::FREE | CollisionMask::PLAYER);
	// FixedJoint joint;
	// auto &bodyA = m_registry.get<RigidBodyComponent>(a);
	// auto &bodyB = m_registry.get<RigidBodyComponent>(b);
	// joint.a = a;
	// joint.b = b;
	// joint.localAnchorA = {0, 0.5, 0};
	// joint.localAnchorB = {0, -0.5, 0};
	// joint.initialRotationOffset =
	// 	QuaternionNormalize(QuaternionMultiply(QuaternionInvert(bodyA.orientation), bodyB.orientation));
	// b.addComponent<FixedJoint>(joint);

	createPlayer(*this, true);
	createEnemy(*this);

	// for (int i = 0; i < 1500; i++)
	// 	setupBlock(*this);
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
