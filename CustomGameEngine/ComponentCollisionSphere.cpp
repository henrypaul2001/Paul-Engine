#include "ComponentCollisionSphere.h"
namespace Engine {
	ComponentCollisionSphere::ComponentCollisionSphere(float sphereRadius, bool defaultCollisionResponse)
	{
		this->sphereRadius = sphereRadius;
		useDefaultCollisionResponse = defaultCollisionResponse;
	}

	ComponentCollisionSphere::~ComponentCollisionSphere()
	{

	}

	void ComponentCollisionSphere::Close()
	{

	}
}