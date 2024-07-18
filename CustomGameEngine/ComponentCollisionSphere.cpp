#include "ComponentCollisionSphere.h"
namespace Engine {
	ComponentCollisionSphere::ComponentCollisionSphere(const ComponentCollisionSphere& old_component)
	{
		this->owner = nullptr;

		this->sphereRadius = old_component.sphereRadius;

		this->EntitiesCheckedThisFrame = old_component.EntitiesCheckedThisFrame;
		this->EntitiesCollidingWith = old_component.EntitiesCollidingWith;

		this->isMovedByCollisions = old_component.isMovedByCollisions;
	}

	ComponentCollisionSphere::ComponentCollisionSphere(float sphereRadius)
	{
		this->sphereRadius = sphereRadius;

		isMovedByCollisions = true;
	}

	ComponentCollisionSphere::~ComponentCollisionSphere()
	{

	}

	void ComponentCollisionSphere::Close()
	{

	}
}