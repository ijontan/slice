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
	Shader shader;
};
