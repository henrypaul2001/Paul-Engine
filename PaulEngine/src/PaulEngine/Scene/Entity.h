#pragma once
#include <entt.hpp>
#include "Scene.h"
#include "PaulEngine/Core/UUID.h"

namespace PaulEngine
{
	class Entity
	{
	public:
		Entity(entt::entity handle = entt::null, Scene* scene = nullptr);

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {
			PE_CORE_ASSERT(!HasComponent<T>(), "Entity already contains component!");
			return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T, typename ...Args>
		T& AddOrReplaceComponent(Args&&... args) {
			return m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent() {
			PE_CORE_ASSERT(HasComponent<T>(), "Entity does not contain component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent() {
			return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent() {
			PE_CORE_ASSERT(HasComponent<T>(), "Entity does not contain component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		bool BelongsToScene(const Ref<Scene>& scene) { return m_Scene == scene.get(); }
		bool IsValid() {
			return (m_Scene && m_Scene->m_Registry.valid(m_EntityHandle));
		}
		operator bool() { return IsValid(); }

		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		bool operator== (const Entity& other) const { return (m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene); }
		bool operator!= (const Entity& other) const { return !(*this == other); }

		const std::string& Tag();
		uint32_t GetID() const { return (uint32_t)m_EntityHandle; }

		const UUID& UUID();

	private:
		friend class std::hash<Entity>;
		entt::entity m_EntityHandle;
		Scene* m_Scene;
	};
}

namespace std
{
	template<>
	struct hash<PaulEngine::Entity>
	{
		size_t operator()(const PaulEngine::Entity& e) const noexcept
		{
			size_t h1 = hash<entt::entity>()(e);
			size_t h2 = hash<PaulEngine::Scene*>()(e.m_Scene);
			return h1 ^ (h2 << 1);
		}
	};
}