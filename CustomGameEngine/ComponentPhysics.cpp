#include "ComponentPhysics.h"
#include <glm/gtc/quaternion.hpp>
namespace Engine {
	ComponentPhysics::ComponentPhysics(float mass, float drag, float surfaceArea, bool gravity)
	{
		SetMass(mass);
		dragCoefficient = drag;
		this->surfaceArea = surfaceArea;
		this->gravity = gravity;

		inertiaTensor = glm::mat3(1.0f);
	}

	ComponentPhysics::ComponentPhysics(float mass, float drag, float surfaceArea)
	{
		SetMass(mass);
		dragCoefficient = drag;
		this->surfaceArea = surfaceArea;
		gravity = true;

		inertiaTensor = glm::mat3(1.0f);
	}

	ComponentPhysics::ComponentPhysics(float mass, float drag)
	{
		SetMass(mass);
		dragCoefficient = drag;
		gravity = true;
		surfaceArea = 1.0f;

		inertiaTensor = glm::mat3(1.0f);
	}

	ComponentPhysics::ComponentPhysics(float mass, bool gravity)
	{
		SetMass(mass);
		this->gravity = gravity;
		dragCoefficient = 1.05f; // cube drag coefficient
		surfaceArea = 1.0f;

		inertiaTensor = glm::mat3(1.0f);
	}

	ComponentPhysics::ComponentPhysics(float mass)
	{
		SetMass(mass);
		gravity = true;
		dragCoefficient = 1.05f; // cube drag coefficient
		surfaceArea = 1.0f;

		inertiaTensor = glm::mat3(1.0f);
	}

	ComponentPhysics::~ComponentPhysics()
	{

	}

	void ComponentPhysics::UpdateInertiaTensor(glm::quat orientation)
	{
		glm::mat3 rotation = glm::mat3_cast(orientation);

		glm::mat3 rotatedTensor = rotation * inertiaTensor * glm::transpose(rotation);

		inertiaTensor = rotatedTensor;
	}

	void ComponentPhysics::Close()
	{
	}
}