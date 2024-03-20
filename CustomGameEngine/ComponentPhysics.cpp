#include "ComponentPhysics.h"
namespace Engine {
	ComponentPhysics::ComponentPhysics(float mass)
	{
		this->mass = mass;
		inverseMass = 1.0f / mass;
	}

	ComponentPhysics::~ComponentPhysics()
	{

	}

	void ComponentPhysics::Close()
	{
	}
}