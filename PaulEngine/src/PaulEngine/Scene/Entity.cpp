#include "pepch.h"
#include "Entity.h"
#include "Components.h"

namespace PaulEngine
{
	Entity::Entity(entt::entity handle, Scene* scene) : m_EntityHandle(handle), m_Scene(scene)
	{

	}

	const std::string& Entity::Tag()
	{
		if (HasComponent<ComponentTag>()) {
			return GetComponent<ComponentTag>().Tag;
		}
	}
}