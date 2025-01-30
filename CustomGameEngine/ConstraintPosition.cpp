#include "ConstraintPosition.h"
#include "ComponentPhysics.h"
#include "ComponentTransform.h"
namespace Engine {
	void ConstraintPosition::UpdateConstraint(EntityManager& ecs, const float deltaTime) const
	{
		const ComponentTransform* transformA = ecs.GetComponent<ComponentTransform>(entityIDA);
		const ComponentTransform* transformB = ecs.GetComponent<ComponentTransform>(entityIDB);

		ComponentPhysics* physicsA = ecs.GetComponent<ComponentPhysics>(entityIDA);
		ComponentPhysics* physicsB = ecs.GetComponent<ComponentPhysics>(entityIDB);

		// At least one of the constrained objects must have a physics component
		if (!physicsA && !physicsB) { return; }

		const glm::vec3 worldSpaceJointPositionA = transformA->GetWorldModelMatrix() * glm::vec4(relativeJointPositionA, 1.0f);
		const glm::vec3 worldSpaceJointPositionB = transformB->GetWorldModelMatrix() * glm::vec4(relativeJointPositionB, 1.0f);
		const glm::vec3 relativePosition = worldSpaceJointPositionA - worldSpaceJointPositionB;
		const float currentDistance = glm::length(relativePosition);
		const float offset = distance - currentDistance;

		const float targetAngleDegrees = 180.0f;

		const glm::vec3 rotationConstraintNormal = glm::normalize(worldSpaceJointPositionA - worldSpaceJointPositionB);

		// Get current angle between them
		const float cosAngle = acos((glm::dot(rotationConstraintNormal, relativeJointPositionA) / (glm::length(rotationConstraintNormal) * glm::length(relativeJointPositionA))));
		float angleDegrees = cosAngle * (180.0f / glm::pi<float>());

		float angleOffset = targetAngleDegrees - angleDegrees;
		if (angleDegrees > 180.0f) {
			angleDegrees = 180.0f - angleDegrees;
		}
		if (abs(offset) > 0.0f) {
			const glm::vec3 direction = glm::normalize(relativePosition);

			glm::vec3 velocityA = glm::vec3(0.0f);
			glm::vec3 velocityB = glm::vec3(0.0f);
			float inverseMassA = 0.0f;
			float inverseMassB = 0.0f;

			if (physicsA) {
				velocityA = physicsA->Velocity();
				inverseMassA = physicsA->InverseMass();
			}
			if (physicsB) {
				velocityB = physicsB->Velocity();
				inverseMassB = physicsB->InverseMass();
			}

			const glm::vec3 relativeVelocity = velocityA - velocityB;
			const float constraintMass = inverseMassA + inverseMassB;

			if (constraintMass > 0.0f) {
				const float constraintStress = glm::dot(relativeVelocity, direction);
				const float biasFactor = this->bias;
				const float bias = -(biasFactor / deltaTime) * offset;

				const float lambda = -(constraintStress + bias) / constraintMass;

				if (physicsA) {
					const glm::vec3 impulseA = direction * lambda;
					physicsA->ApplyLinearImpulse(impulseA);
					physicsA->ApplyAngularImpulse(glm::cross(worldSpaceJointPositionA - transformA->GetWorldPosition(), impulseA * (angleOffset * (angleDegrees / targetAngleDegrees)))); // Angular impulse scales as the angle between the two faces gets larger
				}
				if (physicsB) {
					const glm::vec3 impulseB = -direction * lambda;
					physicsB->ApplyLinearImpulse(impulseB);
					physicsB->ApplyAngularImpulse(glm::cross(worldSpaceJointPositionB - transformB->GetWorldPosition(), impulseB * (angleOffset * (angleDegrees / targetAngleDegrees))));
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