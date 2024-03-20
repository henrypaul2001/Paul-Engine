#include "ComponentPhysics.h"
namespace Engine {
	ComponentPhysics::ComponentPhysics(float mass, bool gravity)
	{
		SetMass(mass);
		this->gravity = gravity;
	}

	ComponentPhysics::ComponentPhysics(float mass)
	{
		SetMass(mass);
		gravity = true;
	}

	ComponentPhysics::~ComponentPhysics()
	{

	}

	void ComponentPhysics::Close()
	{
	}
}