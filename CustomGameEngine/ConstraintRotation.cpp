#include "ConstraintRotation.h"
namespace Engine {
	void ConstraintRotation::UpdateConstraint(float deltaTime)
	{
		glm::vec3 currentOffset = glm::vec3(0.0f);

		glm::vec3 currentRotationA = glm::eulerAngles(objectA.GetTransformComponent()->GetOrientation());
		currentRotationA.x = glm::degrees(currentRotationA.x);
		currentRotationA.y = glm::degrees(currentRotationA.y);
		currentRotationA.z = glm::degrees(currentRotationA.z);

		glm::vec3 currentRotationB = glm::eulerAngles(objectB.GetTransformComponent()->GetOrientation());
		currentRotationB.x = glm::degrees(currentRotationB.x);
		currentRotationB.y = glm::degrees(currentRotationB.y);
		currentRotationB.z = glm::degrees(currentRotationB.z);

		currentOffset = currentRotationB - currentRotationA;
		glm::vec3 offset = maxRotationOffset - currentOffset;

		glm::vec3 relativePosition = objectA.GetTransformComponent()->GetWorldPosition() - objectB.GetTransformComponent()->GetWorldPosition();
		ComponentPhysics* physicsA = objectA.GetPhysicsComponent();
		ComponentPhysics* physicsB = objectB.GetPhysicsComponent();

		glm::vec3 angularVelocityA = glm::vec3(0.0f);
		glm::vec3 angularVelocityB = glm::vec3(0.0f);
		float inverseMassA = 0.0f;
		float inverseMassB = 0.0f;

		if (physicsA != nullptr) {
			angularVelocityA = physicsA->AngularVelocity();
			inverseMassA = physicsA->InverseMass();
		}
		if (physicsB != nullptr) {
			angularVelocityB = physicsB->AngularVelocity();
			inverseMassB = physicsB->InverseMass();
		}

		float constraintMass = inverseMassA + inverseMassB;

		if (constraintMass > 0.0f) {
			glm::vec3 angularImpulse = glm::vec3(0.0f);
			glm::vec3 direction = glm::normalize(relativePosition);
			float constraintStress = glm::dot((angularVelocityB - angularVelocityA), direction);
;			if (abs(currentOffset.x) > abs(maxRotationOffset.x) && controlXRotation) {
				float biasFactor = this->bias;
				float bias = -(biasFactor / deltaTime) * currentOffset.x;

				float lambda = -(constraintStress + bias) / constraintMass;
				angularImpulse.x = lambda;
			}

			if (abs(currentOffset.y) > abs(maxRotationOffset.y) && controlYRotation) {
				float biasFactor = this->bias;
				float bias = -(biasFactor / deltaTime) * currentOffset.y;

				float lambda = -(constraintStress + bias) / constraintMass;
				angularImpulse.y = lambda;
			}

			if (abs(currentOffset.z) > abs(maxRotationOffset.z) && controlZRotation) {
				float biasFactor = this->bias;
				float bias = -(biasFactor / deltaTime) * currentOffset.z;

				float lambda = -(constraintStress + bias) / constraintMass;
				angularImpulse.z = lambda;
			}

			if (physicsA != nullptr) {
				glm::vec3 impulseA = angularImpulse;
				physicsA->AddTorque(impulseA);
			}
			if (physicsB != nullptr) {
				glm::vec3 impulseB = -angularImpulse;
				physicsB->AddTorque(impulseB);
			}
		}
	}
}