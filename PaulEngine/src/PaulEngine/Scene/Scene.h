#pragma once
#include <entt.hpp>

#include "PaulEngine/Asset/Asset.h"
#include "PaulEngine/Core/UUID.h"

#include "PaulEngine/Core/Timestep.h"
#include <PaulEngine/Renderer/Resource/EditorCamera.h>

struct b2WorldId;

namespace PaulEngine
{
	class Entity;
	class Scene : public Asset
	{
	public:
		Scene();
		~Scene();
		
		static Ref<Scene> Copy(Ref<Scene> other);

		// Copies every entity and component from source scene
		void Append(Ref<Scene> source);

		virtual AssetType GetType() const { return AssetType::Scene; }
		const std::string& GetName() const { return m_Name; }

		Entity CreateEntity(const std::string& name = "Entity");
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity");
		Entity DuplicateEntity(Entity entity);
		Entity CopyEntityToScene(Entity srcEntity, Scene* dstScene);
		Entity FindEntityWithUUID(UUID id);
		void DestroyEntity(Entity entity);

		Entity GetPrimaryCameraEntity();

		std::vector<Entity> GetRootEntities();

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
			return m_Registry.view<Owned...>(get);
		}

		template <typename... Owned, typename... Get, typename... Exclude>
		auto View(entt::get_t<Get...> get, entt::exclude_t<Exclude...> exclude) {
			return m_Registry.view<Owned...>(get, exclude);
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
		friend class Prefab;

		void OnPhysics2DStep(Timestep timestep);
		void OnPhysics2DStart();
		void OnPhysics2DStop();

		void DestroyChildren(Entity rootEntity);

		std::string m_Name = "Untitled";

		entt::registry m_Registry;
		std::unordered_map<UUID, Entity> m_EntityMap;

		uint32_t m_ViewportWidth, m_ViewportHeight;

		b2WorldId* m_PhysicsWorld;
	};
}