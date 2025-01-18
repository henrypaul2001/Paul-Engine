#include "ComponentCollision.h"
namespace Engine {
	void ComponentCollision::AddToEntitiesCheckedThisFrame(EntityNew* e)
	{
		EntitiesCheckedThisFrame[e] = e->Name();
	}
	void ComponentCollision::AddToCollisions(EntityNew* e)
	{
		EntitiesCollidingWith[e] = e->Name();
	}
}