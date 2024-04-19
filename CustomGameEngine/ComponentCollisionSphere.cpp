#include "ComponentCollisionSphere.h"
namespace Engine {
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