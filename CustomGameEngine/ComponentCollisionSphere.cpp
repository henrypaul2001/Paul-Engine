#include "ComponentCollisionSphere.h"
namespace Engine {
	ComponentCollisionSphere::ComponentCollisionSphere(float sphereRadius, bool defaultCollisionResponse)
	{
		this->sphereRadius = sphereRadius;
		useDefaultCollisionResponse = defaultCollisionResponse;

		isMovedByCollisions = true;
	}

	ComponentCollisionSphere::~ComponentCollisionSphere()
	{

	}

	void ComponentCollisionSphere::Close()
	{

	}
}