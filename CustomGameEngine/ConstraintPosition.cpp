#include "ConstraintPosition.h"
#include "ComponentPhysics.h"
#include "ComponentTransform.h"
namespace Engine {
	void ConstraintPosition::UpdateConstraint(float deltaTime)
	{
		glm::vec3 worldSpaceJointPositionA = objectA.GetTransformComponent()->GetWorldModelMatrix() * glm::vec4(relativeJointPositionA, 1.0f);
		glm::vec3 worldSpaceJointPositionB = objectB.GetTransformComponent()->GetWorldModelMatrix() * glm::vec4(relativeJointPositionB, 1.0f);
		glm::vec3 relativePosition = worldSpaceJointPositionA - worldSpaceJointPositionB;
		float currentDistance = glm::length(relativePosition);
		float offset = distance - currentDistance;

		float targetAngleDegrees = 180.0f;

		glm::vec3 rotationConstraintNormal = glm::normalize(worldSpaceJointPositionA - worldSpaceJointPositionB);

		// Get current angle between them
		float cosAngle = acos((glm::dot(rotationConstraintNormal, relativeJointPositionA) / (glm::length(rotationConstraintNormal) * glm::length(relativeJointPositionA))));
		float angleDegrees = cosAngle * (180.0f / glm::pi<float>());

		float angleOffset = targetAngleDegrees - angleDegrees;

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
					physicsA->ApplyAngularImpulse(glm::cross(worldSpaceJointPositionA - objectA.GetTransformComponent()->GetWorldPosition(), impulseA * (angleOffset * (angleDegrees / targetAngleDegrees)))); // Angular impulse scales as the angle between the two faces gets larger
				}
				if (physicsB != nullptr) {
					glm::vec3 impulseB = -direction * lambda;
					physicsB->ApplyLinearImpulse(impulseB);
					physicsB->ApplyAngularImpulse(glm::cross(worldSpaceJointPositionB - objectB.GetTransformComponent()->GetWorldPosition(), impulseB * (angleOffset * (angleDegrees / targetAngleDegrees))));
				}
			}
		}

		/*
		if (abs(angleOffset) > 5.0f) {
			glm::vec3 direction = glm::normalize(relativePosition);

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

			glm::vec3 relativeAngular = angularVelocityA - angularVelocityB;
			float constraintMass = inverseMassA + inverseMassB;

			glm::vec3 relativeAngularA = glm::vec3();
			glm::vec3 relativeAngularB = glm::vec3();

			if (physicsA != nullptr) {
				relativeAngularA = glm::cross(angularVelocityA, relativeJointPositionA);
			}
			if (physicsB != nullptr) {
				relativeAngularB = glm::cross(angularVelocityB, relativeJointPositionB);
			}

			//float impulseForce = glm::dot(contactVelocity, contact.normal);

			glm::vec3 inertiaA = glm::vec3(0.0f);
			glm::vec3 inertiaB = glm::vec3(0.0f);
			if (physicsA != nullptr) { inertiaA = glm::cross(physicsA->InertiaTensor() * glm::cross(relativeJointPositionA, direction), relativeJointPositionA); }
			if (physicsB != nullptr) { inertiaB = glm::cross(physicsB->InertiaTensor() * glm::cross(relativeJointPositionB, direction), relativeJointPositionB); }

			float angularEffect = glm::dot(inertiaA + inertiaB, direction);
			constraintMass += angularEffect;
			if (constraintMass > 0.0f) {
				float constraintStress = glm::dot(relativeAngular, -direction);
				float biasFactor = this->bias;
				//float biasFactor = 0.0000005f;
				float bias = -(biasFactor / deltaTime) * angleOffset;

				float lambda = -(constraintStress + bias) / constraintMass;

				if (physicsA != nullptr) {
					glm::vec3 impulseA = -direction * lambda;
					//physicsA->ApplyAngularImpulse(glm::cross(objectA.GetTransformComponent()->GetWorldPosition() - worldSpaceJointPositionA, impulseA));
				}
				if (physicsB != nullptr) {
					glm::vec3 impulseB = direction * lambda;
					//physicsB->ApplyAngularImpulse(glm::cross(objectB.GetTransformComponent()->GetWorldPosition() - worldSpaceJointPositionB , impulseB));
				}
			}
		}
		*/
	}
}