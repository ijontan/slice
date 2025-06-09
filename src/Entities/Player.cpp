#include "Entities/Player.hpp"
#include "Entity.hpp"
#include "PlayerState.hpp"
#include "RigidBody.hpp"
#include "Scene.hpp"
#include "Shape.hpp"
#include "raylib.h"
#include "raymath.h"

void createPlayer(Scene &scene, bool active)
{
	Camera cam = {};
	cam.position = (Vector3){0.0f, 2.0f, 4.0f};
	cam.target = (Vector3){0.0f, 2.0f, 0.0f};
	cam.up = (Vector3){0.0f, 1.0f, 0.0f};
	cam.fovy = 60.0f;
	cam.projection = CAMERA_PERSPECTIVE;

	Entity player = scene.createEntity();
	player.addComponent<Camera>(cam);

	Mesh mesh = GenMeshCube(0.5, 0.5, 0.5);
	Model model = LoadModelFromMesh(mesh);
	// set shader to first (and only) material.
	model.materials->shader = scene.shader;
	model.transform = MatrixTranslate(cam.target.x, cam.target.y, cam.target.z);
	player.addComponent<BoxComponent>(model, PURPLE);

	PlayerState state = {};
	state.active = active;
	state.camLock = false;
	player.addComponent<PlayerState>(state);

	OBB obb = {};
	obb.halfSize = Vector3Scale({0.5, 0.5, 0.5}, 0.5);
	RigidBodyComponent body(10, cam.target, (Vector3){}, (Vector3){}, CollisionMask::PLAYER, CollisionMask::ALL,
							(Matrix){}, obb, QuaternionIdentity());
	player.addComponent<RigidBodyComponent>(body);
}
