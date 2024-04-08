#include "ConstraintPosition.h"
#include "ComponentPhysics.h"
#include "ComponentTransform.h"
namespace Engine {
	void ConstraintPosition::UpdateConstraint(float deltaTime)
	{
		glm::vec3 relativePosition = (objectA.GetTransformComponent()->GetWorldPosition() + relativeJointPositionA) - (objectB.GetTransformComponent()->GetWorldPosition() + relativeJointPositionB);
		float currentDistance = glm::length(relativePosition);
		float offset = distance - currentDistance;

		//float targetAngleDegrees = 180.0f;

		//glm::vec3 rotationConstraintNormalA = glm::vec3(0.0f, 1.0f, 0.0f); // up vector
		//glm::vec3 rotationConstrainNormalB = glm::vec3(0.0f, 1.0f, 0.0f);

		// Transform into world space

		// Get current angle between them


		if (abs(offset) > 0.0f) {
			glm::vec3 direction = glm::normalize(relativePosition);

			ComponentPhysics* physicsA = objectA.GetPhysicsComponent();
			ComponentPhysics* physicsB = objectB.GetPhysicsComponent();

			glm::vec3 velocityA = glm::vec3(0.0f);
			glm::vec3 velocityB = glm::vec3(0.0f);
			float inverseMassA = 0.0f;
			float inverseMassB = 0.0f;

			if (physicsA != nullptr) {
				velocityA = physicsA->Velocity();
				inverseMassA = physicsA->InverseMass();
			}
			if (physicsB != nullptr) {
				velocityB = physicsB->Velocity();
				inverseMassB = physicsB->InverseMass();
			}

			glm::vec3 relativeVelocity = velocityA - velocityB;
			float constraintMass = inverseMassA + inverseMassB;

			if (constraintMass > 0.0f) {
				float constraintStress = glm::dot(relativeVelocity, direction);
				float biasFactor = this->bias;
				float bias = -(biasFactor / deltaTime) * offset;

				float lambda = -(constraintStress + bias) / constraintMass;

				if (physicsA != nullptr) {
					glm::vec3 impulseA = direction * lambda;
					physicsA->ApplyLinearImpulse(impulseA);
					//physicsA->SetTorque(impulseA);
					// Apply angular impulse at relative joint position A in direction of constraint normal
				}
				if (physicsB != nullptr) {
					glm::vec3 impulseB = -direction * lambda;
					physicsB->ApplyLinearImpulse(impulseB);
					//physicsB->SetTorque(impulseB);
				}
			}
		}
	}
}