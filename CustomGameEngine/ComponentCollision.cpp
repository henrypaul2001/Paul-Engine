#include "ComponentCollision.h"
namespace Engine {
	void ComponentCollision::AddToEntitiesCheckedThisFrame(const unsigned int e, const std::string& name)
	{
		EntitiesCheckedThisFrame[e] = name;
	}
	void ComponentCollision::AddToCollisions(const unsigned int e, const std::string& name)
	{
		EntitiesCollidingWith[e] = name;
	}
}