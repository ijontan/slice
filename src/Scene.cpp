
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
#include <iostream>
#include <numeric>
#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

Scene::Scene()
{
	BlockFactory blockFactory(*this);
	// Load basic lighting shader
	shader = LoadShader("shaders/lighting.vs", "shaders/lighting.fs");
	// Get some required shader locations
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
	// NOTE: "matModel" location name is automatically assigned on shader loading,
	// no need to get the location again if using that uniform name
	// shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");

	// Ambient light level (some basic lighting)
	int ambientLoc = GetShaderLocation(shader, "ambient");
	SetShaderValue(shader, ambientLoc, (float[4]){0.2f, 0.2f, 0.2f, 1.0f}, SHADER_UNIFORM_VEC4);

	// Create lights
	Light lights[MAX_LIGHTS] = {};
	lights[0] = CreateLight(LIGHT_POINT, (Vector3){5000, 0, 0}, Vector3Zero(), Color({230, 240, 255, 255}), shader);
	lights[1] = CreateLight(LIGHT_POINT, (Vector3){-5000, 0, 0}, Vector3Zero(), Color({40, 30, 20, 255}), shader);

	createPlayer(*this, true);
	// createEnemy(*this);
	// createEnemy(*this);
	// createEnemy(*this);
	// createEnemy(*this);

	for (int i = 0; i < 2000; i++)
	{
		blockFactory.randomize();
		blockFactory.generateBlock();
	}
}

Scene::~Scene(void)
{
	UnloadShader(shader);
	auto view = m_registry.view<BoxComponent>();
	for (auto entity : view)
	{
		BoxComponent &box = m_registry.get<BoxComponent>(entity);
		UnloadModel(box.model);
	}
}

Entity Scene::createEntity()
{
	return Entity(m_registry.create(), this);
}

void Scene::render()
{
	auto playerView = m_registry.view<Camera>();

	playerSystem(m_registry);
	enemyRegenBodySystem(m_registry, *this);
	BeginDrawing();
	BeginShaderMode(shader);

	ClearBackground(BLACK);

	{
		animateBlockSpawn(m_registry);
		stepPhysicSimulation(m_registry);
		renderBlocks(m_registry);
	}

	EndShaderMode();
	for (auto entity : playerView)
	{
		Camera &camera = m_registry.get<Camera>(entity);
		BeginMode3D(camera);
		DrawSphere({700, 0, 0}, 400, Color({180, 200, 255, 255}));
		DrawSphere({-500, 0, 0}, 30, Color({255, 210, 200, 255}));
		EndMode3D();
	}
	EndDrawing();
}
