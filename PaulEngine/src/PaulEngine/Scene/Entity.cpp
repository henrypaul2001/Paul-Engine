#include "pepch.h"
#include "Entity.h"

namespace PaulEngine
{
	Entity::Entity(entt::entity handle, Scene* scene) : m_EntityHandle(handle), m_Scene(scene)
	{

	}
}