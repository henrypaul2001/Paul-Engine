#include "ComponentPhysics.h"
namespace Engine {
	ComponentPhysics::ComponentPhysics(float mass, float drag, float surfaceArea)
	{
		SetMass(mass);
		dragCoefficient = drag;
		this->surfaceArea = surfaceArea;
		gravity = true;
	}

	ComponentPhysics::ComponentPhysics(float mass, float drag)
	{
		SetMass(mass);
		dragCoefficient = drag;
		gravity = true;
		surfaceArea = 1.0f;
	}

	ComponentPhysics::ComponentPhysics(float mass, bool gravity)
	{
		SetMass(mass);
		this->gravity = gravity;
		dragCoefficient = 1.05f; // cube drag coefficient
		surfaceArea = 1.0f;
	}

	ComponentPhysics::ComponentPhysics(float mass)
	{
		SetMass(mass);
		gravity = true;
		dragCoefficient = 1.05f; // cube drag coefficient
		surfaceArea = 1.0f;
	}

	ComponentPhysics::~ComponentPhysics()
	{

	}

	void ComponentPhysics::Close()
	{
	}
}