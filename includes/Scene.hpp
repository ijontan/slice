#pragma once
#include "entt.hpp"
#include "raylib.h"

struct Entity;

class Scene
{
public:
	Scene();
	~Scene(void);

	Entity createEntity();
	void render();

	entt::registry m_registry;

	Camera cam;
	Shader shader;
	float runTime;
	int viewEyeLoc;
	int viewCenterLoc;
	int runTimeLoc;
	bool cam_lock = false;
};
