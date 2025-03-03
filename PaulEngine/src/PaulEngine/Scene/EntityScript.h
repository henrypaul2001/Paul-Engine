#pragma once
#include "Entity.h"

namespace PaulEngine
{
	class EntityScript
	{
	public:
		virtual ~EntityScript() {}

		template<typename T>
		T& GetComponent() { return m_Entity.GetComponent<T>(); }

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) { return m_Entity.AddComponent<T>(std::forward<Args>(args)...); }

		template<typename T>
		bool HasComponent() { return m_Entity.HasComponent<T>(); }

		template<typename T>
		void RemoveComponent() { return m_Entity.RemoveComponent<T>(); }

		bool IsValid() { return m_Entity.IsValid(); }
		operator bool() { return IsValid(); }

		const std::string& Tag() { return m_Entity.Tag(); }

	protected:
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(const Timestep timestep) {}

	protected:
		Entity m_Entity;
		friend class Scene;
	};
}