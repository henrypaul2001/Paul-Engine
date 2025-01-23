#include "ConstraintRotation.h"
namespace Engine {
	void ConstraintRotation::UpdateConstraint(EntityManagerNew& ecs, const float deltaTime) const
	{
		const ComponentTransform* transformA = ecs.GetComponent<ComponentTransform>(objectA.ID());
		const ComponentTransform* transformB = ecs.GetComponent<ComponentTransform>(objectB.ID());

		ComponentPhysics* physicsA = ecs.GetComponent<ComponentPhysics>(objectA.ID());
		ComponentPhysics* physicsB = ecs.GetComponent<ComponentPhysics>(objectB.ID());

		// At least one of the constrained objects must have a physics component
		if (!physicsA && !physicsB) { return; }

		glm::vec3 currentOffset = glm::vec3(0.0f);

		glm::vec3 currentRotationA = glm::eulerAngles(transformA->GetOrientation());
		currentRotationA.x = glm::degrees(currentRotationA.x);
		currentRotationA.y = glm::degrees(currentRotationA.y);
		currentRotationA.z = glm::degrees(currentRotationA.z);

		glm::vec3 currentRotationB = glm::eulerAngles(transformB->GetOrientation());
		currentRotationB.x = glm::degrees(currentRotationB.x);
		currentRotationB.y = glm::degrees(currentRotationB.y);
		currentRotationB.z = glm::degrees(currentRotationB.z);

		currentOffset = currentRotationB - currentRotationA;
		const glm::vec3 offset = maxRotationOffset - currentOffset;

		const glm::vec3 relativePosition = transformA->GetWorldPosition() - transformB->GetWorldPosition();

		glm::vec3 angularVelocityA = glm::vec3(0.0f);
		glm::vec3 angularVelocityB = glm::vec3(0.0f);
		float inverseMassA = 0.0f;
		float inverseMassB = 0.0f;

		if (physicsA) {
			angularVelocityA = physicsA->AngularVelocity();
			inverseMassA = physicsA->InverseMass();
		}
		if (physicsB) {
			angularVelocityB = physicsB->AngularVelocity();
			inverseMassB = physicsB->InverseMass();
		}

		const float constraintMass = inverseMassA + inverseMassB;

		if (constraintMass > 0.0f) {
			glm::vec3 angularImpulse = glm::vec3(0.0f);
			const glm::vec3 direction = glm::normalize(relativePosition);
			const float constraintStress = glm::dot((angularVelocityB - angularVelocityA), direction);
;			if (abs(currentOffset.x) > abs(maxRotationOffset.x) && controlXRotation) {
				const float biasFactor = this->bias;
				const float bias = -(biasFactor / deltaTime) * currentOffset.x;

				const float lambda = -(constraintStress + bias) / constraintMass;
				angularImpulse.x = lambda;
			}

			if (abs(currentOffset.y) > abs(maxRotationOffset.y) && controlYRotation) {
				const float biasFactor = this->bias;
				const float bias = -(biasFactor / deltaTime) * currentOffset.y;

				const float lambda = -(constraintStress + bias) / constraintMass;
				angularImpulse.y = lambda;
			}

			if (abs(currentOffset.z) > abs(maxRotationOffset.z) && controlZRotation) {
				const float biasFactor = this->bias;
				const float bias = -(biasFactor / deltaTime) * currentOffset.z;

				const float lambda = -(constraintStress + bias) / constraintMass;
				angularImpulse.z = lambda;
			}

			if (physicsA != nullptr) {
				const glm::vec3 impulseA = angularImpulse;
				physicsA->AddTorque(impulseA);
			}
			if (physicsB != nullptr) {
				const glm::vec3 impulseB = -angularImpulse;
				const glm::vec3 torqueImpulseB = impulseB;
				physicsB->AddTorque(impulseB);

				// Angular
				/*
				physicsB->UpdateInertiaTensor(objectB.GetTransformComponent()->GetOrientation());

				glm::vec3 angularAcceleration = physicsB->InverseInertiaTensor() * torqueImpulseB;
				glm::vec3 angularVelocity = physicsB->AngularVelocity();

				angularVelocity += angularAcceleration * deltaTime;
				physicsB->SetAngularVelocity(angularVelocity);

				// Angular velocity
				glm::quat orientation = objectB.GetTransformComponent()->GetOrientation();
				angularVelocity = physicsB->AngularVelocity();

				orientation = orientation + (glm::quat(glm::vec3(angularVelocity * deltaTime * 0.5f)) * orientation);
				orientation = glm::normalize(orientation);

				objectB.GetTransformComponent()->SetOrientation(orientation);
				*/
			}
		}
	}
}