#pragma once
#include "Scene.hpp"
#include <utility>

struct Entity
{
	Entity() = default;
	Entity(entt::entity handle, Scene *scene) : m_entityHandle(handle), scene(scene)
	{
	}

	template <typename T, typename... Arg>
	T &addComponent(Arg &&...args)
	{
		return scene->m_registry.emplace<T>(m_entityHandle, std::forward<Arg>(args)...);
	}

	template <typename T>
	T &getComponent()
	{
		return scene->m_registry.get<T>(m_entityHandle);
	}

	template <typename T>
	void removeComponent()
	{
		return scene->m_registry.remove<T>(m_entityHandle);
	}

	operator entt::entity()
	{
		return m_entityHandle;
	}

	operator entt::entity() const
	{
		return m_entityHandle;
	}

private:
	entt::entity m_entityHandle{0};
	Scene *scene = nullptr;
};
