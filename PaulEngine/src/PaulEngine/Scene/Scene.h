#pragma once
#include <entt.hpp>
//#include "Components.h"

#include "PaulEngine/Core/Timestep.h"

namespace PaulEngine
{
	class Entity;
	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = "Entity");

		void OnUpdate(Timestep timestep);
		void OnViewportResize(uint32_t width, uint32_t height);

	private:
		friend class Entity;
		friend class SceneHierarchyPanel;

		entt::registry m_Registry;

		uint32_t m_ViewportWidth, m_ViewportHeight;
	};
}