#include "raylib.h"
//
#include "RigidBody.hpp"
#include "Shape.hpp"
#include "Systems/RenderSystem.hpp"
#include "entt.hpp"
#include "raymath.h"

void renderBlocks(entt::registry &registry, Camera &camera)
{
	// auto view = registry.view<BoxComponent, RigidBodyComponent>();
	auto view = registry.view<BoxComponent>();
	BeginMode3D(camera);
	for (auto entity : view)
	{
		auto box = registry.get<BoxComponent>(entity);
		// auto rigidBody = registry.get<RigidBodyComponent>(entity);
		// Matrix translate = MatrixTranslate(rigidBody.center.x, rigidBody.center.y, rigidBody.center.z);
		// Matrix model = MatrixMultiply(rigidBody.orientation, translate);
		// Vector3 axis;
		// float angle;
		// QuaternionToAxisAngle(rigidBody.orientation, &axis, &angle);
		// angle = angle / PI * 180;
		// DrawModelEx(box.model, rigidBody.center, axis, angle, {1.0f, 1.0f, 1.0f}, WHITE);
		// DrawModelWiresEx(box.model, rigidBody.center, axis, angle, {1.0f, 1.0f, 1.0f}, BLACK);
		DrawModel(box.model, Vector3Zero(), 1, WHITE);
		DrawModelWires(box.model, Vector3Zero(), 1, BLACK);
		// DrawBoundingBox(box.boundingBox, BLACK);
		// DrawMesh(box.mesh, box.material, model);
	}
	EndMode3D();
}
