#pragma once
#include <entt.hpp>

#include "PaulEngine/Asset/Asset.h"
#include "PaulEngine/Core/UUID.h"

#include "PaulEngine/Core/Timestep.h"
#include <PaulEngine/Renderer/EditorCamera.h>

struct b2WorldId;

namespace PaulEngine
{
	class Entity;
	class Scene : public Asset
	{
	public:
		static Ref<Scene> Create();

		Scene();
		~Scene();
		
		static Ref<Scene> Copy(Ref<Scene> other);

		virtual AssetType GetType() const { return AssetType::Scene; }

		Entity CreateEntity(const std::string& name = "Entity");
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity");
		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);

		Entity GetPrimaryCameraEntity();

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateRuntime(Timestep timestep);
		void OnUpdateSimulation(Timestep timestep, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

		template <typename... Components>
		auto View() {
			return m_Registry.view<Components...>();
		}

		template <typename... Owned, typename... Get>
		auto View(entt::get_t<Get...> get) {
			return m_Registry.view<Components...>(get);
		}

		template <typename... Owned, typename... Get, typename... Exclude>
		auto View(entt::get_t<Get...> get, entt::exclude_t<Exclude...> exclude) {
			return m_Registry.view<Components...>(get, exclude);
		}

		template <typename... Components>
		auto Group() {
			return m_Registry.group<Components...>();
		}

		template <typename... Owned, typename... Get>
		auto Group(entt::get_t<Get...> get) {
			return m_Registry.group<Owned...>(get);
		}

		template <typename... Owned, typename... Get, typename... Exclude>
		auto Group(entt::get_t<Get...> get, entt::exclude_t<Exclude...> exclude) {
			return m_Registry.group<Owned...>(get, exclude);
		}

	private:
		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;

		void OnPhysics2DStep(Timestep timestep);
		void OnPhysics2DStart();
		void OnPhysics2DStop();

		entt::registry m_Registry;

		uint32_t m_ViewportWidth, m_ViewportHeight;

		b2WorldId* m_PhysicsWorld;
	};
}