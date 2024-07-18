#include "ComponentPhysics.h"
#include <glm/gtc/quaternion.hpp>
namespace Engine {
	ComponentPhysics::ComponentPhysics(const ComponentPhysics& old_component)
	{
		this->owner = nullptr;

		this->gravity = old_component.gravity;

		this->surfaceArea = old_component.surfaceArea;
		this->dragCoefficient = old_component.dragCoefficient;
		this->inverseMass = old_component.inverseMass;
		this->mass = old_component.mass;
		this->elasticity = old_component.elasticity;

		this->velocity = old_component.velocity;

		this->angularVelocity = old_component.angularVelocity;
		this->torque = old_component.torque;
		this->inertiaTensor = old_component.inertiaTensor;

		this->inverseInertiaTensor = old_component.inverseInertiaTensor;
		this->inverseInertia = old_component.inverseInertia;

		this->force = old_component.force;
	}

	ComponentPhysics::ComponentPhysics(float mass, float drag, float surfaceArea, float elasticity, bool gravity, bool cuboidInertiaTensor)
	{
		SetMass(mass);
		dragCoefficient = drag;
		this->surfaceArea = surfaceArea;
		this->gravity = gravity;
		this->elasticity = elasticity;

		if (!cuboidInertiaTensor) {
			float radius = surfaceArea; // temp
			float I = 2.5f * inverseMass / (radius * radius);

			inverseInertia = glm::vec3(I);
		}
		else {
			glm::vec3 dimensions = glm::vec3(surfaceArea);
			glm::vec3 dimsSqr = dimensions * dimensions;

			//inverseInertia.x = (12.0f * inverseMass) / (dimsSqr.y + dimsSqr.z);
			//inverseInertia.y = (12.0f * inverseMass) / (dimsSqr.x + dimsSqr.z);
			//inverseInertia.z = (12.0f * inverseMass) / (dimsSqr.x + dimsSqr.y);

			inverseInertia.x = ((1.0f / 6.0f) * mass) * (dimsSqr.y + dimsSqr.z);
			inverseInertia.y = ((1.0f / 6.0f) * mass) * (dimsSqr.x + dimsSqr.z);
			inverseInertia.z = ((1.0f / 6.0f) * mass) * (dimsSqr.x + dimsSqr.y);
		}
	}

	ComponentPhysics::~ComponentPhysics()
	{

	}

	void ComponentPhysics::UpdateInertiaTensor(glm::quat orientation)
	{
		glm::mat3 inverseOrientation = glm::mat3_cast(glm::conjugate(orientation));
		glm::mat3 rotation = glm::mat3_cast(orientation);

		glm::mat3 scaled = glm::mat3();
		scaled[0][0] = inverseInertia.x;
		scaled[1][1] = inverseInertia.y;
		scaled[2][2] = inverseInertia.z;

		inverseInertiaTensor = rotation * scaled * inverseOrientation;
	}

	void ComponentPhysics::Close()
	{
	}
}