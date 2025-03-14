#pragma once
#include <entt.hpp>
//#include "Components.h"

#include "PaulEngine/Core/Timestep.h"
#include <PaulEngine/Renderer/EditorCamera.h>

struct b2WorldId;

namespace PaulEngine
{
	class Entity;
	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = "Entity");
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnUpdateRuntime(Timestep timestep);
		void OnUpdateOffline(Timestep timestep, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

	private:
		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;

		entt::registry m_Registry;

		uint32_t m_ViewportWidth, m_ViewportHeight;

		b2WorldId* m_PhysicsWorld;
	};
}