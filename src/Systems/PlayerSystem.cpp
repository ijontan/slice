#include "Systems/PlayerSystem.hpp"
#include "Player.hpp"
#include "RigidBody.hpp"
#include "entt.hpp"
#include "raylib.h"

void playerSystem(entt::registry &registry)
{
	auto view = registry.view<Camera, RigidBodyComponent, PlayerState>();

	for (auto entity : view)
	{
		PlayerState &state = registry.get<PlayerState>(entity);
		if (!state.active)
			continue;

		Camera &cam = registry.get<Camera>(entity);
		RigidBodyComponent &rigidBody = registry.get<RigidBodyComponent>(entity);

		if (IsKeyPressed(KEY_E))
		{
			state.camLock = !state.camLock;
			if (state.camLock)
				EnableCursor();
			else
				DisableCursor();
		}

		if (!state.camLock)
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
		rigidBody.center = cam.target;
	}
}
