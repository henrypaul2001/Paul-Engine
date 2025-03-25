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
		Scene();
		~Scene();
		
		static Ref<Scene> Copy(Ref<Scene> other);

		virtual AssetType GetType() const { return AssetType::Scene; }

		Entity CreateEntity(const std::string& name = "Entity");
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity");
		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);

		template<typename... Components>
		auto GetAllEntitiesWith() {
			return m_Registry.view<Components...>();
		}
		Entity GetPrimaryCameraEntity();

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateRuntime(Timestep timestep);
		void OnUpdateSimulation(Timestep timestep, EditorCamera& camera);
		void OnUpdateOffline(Timestep timestep, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

	private:
		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;

		void OnPhysics2DStart();
		void OnPhysics2DStop();

		void RenderScene(EditorCamera& camera);

		entt::registry m_Registry;

		uint32_t m_ViewportWidth, m_ViewportHeight;

		b2WorldId* m_PhysicsWorld;
	};
}