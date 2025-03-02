#pragma once
#include <entt.hpp>
#include "Components.h"

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

	private:
		friend class Entity;

		entt::registry m_Registry;
	};
}