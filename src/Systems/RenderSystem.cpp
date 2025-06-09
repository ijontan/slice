#include "raylib.h"
//
#include "RigidBody.hpp"
#include "Shape.hpp"
#include "Systems/RenderSystem.hpp"
#include "entt.hpp"
#include "raymath.h"

void renderBlocks(entt::registry &registry)
{
	auto playerView = registry.view<Camera>();
	auto view = registry.view<BoxComponent>();
	for (auto entity : playerView)
	{
		Camera &camera = registry.get<Camera>(entity);
		BeginMode3D(camera);
		for (auto entity : view)
		{
			auto box = registry.get<BoxComponent>(entity);
			DrawModel(box.model, Vector3Zero(), box.scale, box.color);
		}
		EndMode3D();
	}
}
