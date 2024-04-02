#include "ComponentCollision.h"
#include "Entity.h"
namespace Engine {
	void ComponentCollision::AddToEntitiesCheckedThisFrame(Entity* e)
	{
		EntitiesCheckedThisFrame[e] = e->Name();
	}
	void ComponentCollision::AddToCollisions(Entity* e)
	{
		EntitiesCollidingWith[e] = e->Name();
	}
}