#pragma once
#include "Component.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/quaternion_float.hpp>
namespace Engine {
    class ComponentPhysics : public Component
    {
	public:
		ComponentPhysics(const ComponentPhysics& old_component);
		ComponentPhysics(float mass = 10.0f, float drag = 1.05f, float surfaceArea = 1.0f, float elasticity = 0.5f, bool gravity = true, bool cuboidIntertiaTensor = false);
		~ComponentPhysics();

		Component* Copy() override { return new ComponentPhysics(*this); }

		void ClearForces() { force = glm::vec3(0.0f); }
		void AddForce(glm::vec3 force) { this->force += force; }
		void AddForce(glm::vec3 force, glm::vec3 forcePositionLocal) {
			this->force += force;
			torque += glm::cross(forcePositionLocal, force);
		}

		glm::vec3 Velocity() { return velocity; }
		glm::vec3 Force() { return force; }
		float InverseMass() { return inverseMass; }
		float RawMass() { return mass; }

		void SetMass(float newMass) {
			mass = newMass;
			UpdateInverseMass();
		}

		void SetVelocity(glm::vec3 newVelocity) { velocity = newVelocity; }
		void SetAngularVelocity(glm::vec3 newVelocity) { angularVelocity = newVelocity; }

		void ApplyLinearImpulse(glm::vec3 force) { velocity += force * inverseMass; }
		void ApplyAngularImpulse(glm::vec3 angularForce) { angularVelocity += inverseInertiaTensor * angularForce; }

		void SetTorque(glm::vec3 newTorque) { torque = newTorque; }
		void AddTorque(glm::vec3 newTorque) { torque += newTorque; }

		bool Gravity() { return gravity; }
		void Gravity(bool gravity) { this->gravity = gravity; }

		glm::mat3 InertiaTensor() { return inertiaTensor; }
		glm::vec3 Torque() { return torque; }
		glm::vec3 AngularVelocity() { return angularVelocity; }

		glm::mat3 InverseInertiaTensor() { return inverseInertiaTensor; }
		glm::vec3 InverseInertia() { return inverseInertia; }

		void UpdateInertiaTensor(glm::quat orientation);

		float DragCoefficient() { return dragCoefficient; }
		void DrafCoefficient(float newDrag) { dragCoefficient = newDrag; }

		float SurfaceArea() { return surfaceArea; }
		void SurfaceArea(float surfaceArea) { this->surfaceArea = surfaceArea; }

		float Elasticity() { return elasticity; }
		float Elasticity(float elasticity) { this->elasticity = elasticity; }

		ComponentTypes ComponentType() override { return COMPONENT_PHYSICS; }
		void Close() override;
	private:
		void UpdateInverseMass() { inverseMass = 1.0f / mass; }

		bool gravity;

		float surfaceArea;
		float dragCoefficient;
		float inverseMass;
		float mass;
		float elasticity;
		
		glm::vec3 velocity;
		
		glm::vec3 angularVelocity;
		glm::vec3 torque;
		glm::mat3 inertiaTensor;

		glm::mat3 inverseInertiaTensor;
		glm::vec3 inverseInertia;

		glm::vec3 force;
    };
}