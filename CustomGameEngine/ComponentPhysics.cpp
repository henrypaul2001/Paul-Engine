#include "ComponentPhysics.h"
#include <glm/gtc/quaternion.hpp>
namespace Engine {
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
			float maxX = surfaceArea;
			float maxY = surfaceArea;
			float maxZ = surfaceArea;

			glm::vec3 dimensions = glm::vec3(surfaceArea); // temp
			glm::vec3 dimensionsSquared = dimensions * dimensions;

			inverseInertia.x = (12.0f * inverseMass) / (dimensionsSquared.y + dimensionsSquared.z);
			inverseInertia.y = (12.0f * inverseMass) / (dimensionsSquared.x + dimensionsSquared.z);
			inverseInertia.z = (12.0f * inverseMass) / (dimensionsSquared.x + dimensionsSquared.y);
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