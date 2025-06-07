#include "Systems/PlayerSystem.hpp"
#include "PlayerState.hpp"
#include "RigidBody.hpp"
#include "entt.hpp"
#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <ostream>

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
		Vector3 moveDirection = Vector3RotateByQuaternion({1, 0, 0}, rigidBody.orientation);
		Vector3 camUp = Vector3RotateByQuaternion({0, 1, 0}, rigidBody.orientation);
		Vector3 camRight = Vector3RotateByQuaternion({0, 0, 1}, rigidBody.orientation);

		static float amount = 0;
		if (IsKeyDown(KEY_LEFT_SHIFT))
			rigidBody.force = Vector3Scale(Vector3Negate(rigidBody.velocity), 20);

		bool boost = false;
		if (IsKeyDown(KEY_W) || IsKeyDown(KEY_S) || IsKeyDown(KEY_A) || IsKeyDown(KEY_D))
		{
			boost = IsKeyDown(KEY_SPACE);
			float scale = boost ? 900.0f : 300.0f;
			amount = 1;
			float value = (IsKeyDown(KEY_W) * scale) - (IsKeyDown(KEY_S) * scale);
			rigidBody.force = Vector3Scale(moveDirection, value);
		}
		if (!state.camLock)
		{
			Vector2 mouseDelta = GetMouseDelta();
			rigidBody.orientation =
				QuaternionMultiply(QuaternionFromAxisAngle(camRight, -mouseDelta.y * 0.001), rigidBody.orientation);
			rigidBody.orientation =
				QuaternionMultiply(QuaternionFromAxisAngle(camUp, -mouseDelta.x * 0.001), rigidBody.orientation);
		}
		if (amount > 0)
			amount -= 0.05;

		float maxSpeed = boost ? 40.0f : 20.0f;
		rigidBody.velocity = Vector3ClampValue(rigidBody.velocity, -maxSpeed, maxSpeed);

		cam.target = rigidBody.center;
		float camDistance = boost ? 12.0f : 7.0f;
		Vector3 positionTarget = Vector3Add(cam.target, Vector3Scale(moveDirection, -camDistance));
		cam.position = Vector3Lerp(positionTarget, cam.position, amount);
		cam.up = camUp;
	}
}
