#include "ComponentPhysics.h"
namespace Engine {
	ComponentPhysics::ComponentPhysics(const float mass, const float drag, const float surfaceArea, const float elasticity, const bool gravity, const bool cuboidInertiaTensor) : dragCoefficient(drag), surfaceArea(surfaceArea), gravity(gravity), elasticity(elasticity)
	{
		SetMass(mass);

		if (cuboidInertiaTensor) {
			const glm::vec3 dimensions = glm::vec3(surfaceArea);
			const glm::vec3 dimsSqr = dimensions * dimensions;

			//inverseInertia.x = (12.0f * inverseMass) / (dimsSqr.y + dimsSqr.z);
			//inverseInertia.y = (12.0f * inverseMass) / (dimsSqr.x + dimsSqr.z);
			//inverseInertia.z = (12.0f * inverseMass) / (dimsSqr.x + dimsSqr.y);

			inverseInertia.x = ((1.0f / 6.0f) * mass) * (dimsSqr.y + dimsSqr.z);
			inverseInertia.y = ((1.0f / 6.0f) * mass) * (dimsSqr.x + dimsSqr.z);
			inverseInertia.z = ((1.0f / 6.0f) * mass) * (dimsSqr.x + dimsSqr.y);
		}
		else {
			const float radius = surfaceArea; // temp
			const float I = 2.5f * inverseMass / (radius * radius);
			inverseInertia = glm::vec3(I);
		}

		UpdateInertiaTensor(glm::quat());
	}

	ComponentPhysics::~ComponentPhysics() {}

	void ComponentPhysics::UpdateInertiaTensor(const glm::quat& orientation)
	{
		const glm::mat3 inverseOrientation = glm::mat3_cast(glm::conjugate(orientation));
		const glm::mat3 rotation = glm::mat3_cast(orientation);

		glm::mat3 scaled = glm::mat3();
		scaled[0][0] = inverseInertia.x;
		scaled[1][1] = inverseInertia.y;
		scaled[2][2] = inverseInertia.z;

		inverseInertiaTensor = rotation * scaled * inverseOrientation;
	}
}