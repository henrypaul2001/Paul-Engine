#pragma once
#include "Component.h"
#include <glm/ext/vector_float3.hpp>
namespace Engine {
    class ComponentPhysics : public Component
    {
	public:
		ComponentPhysics(float mass, bool gravity);
		ComponentPhysics(float mass);
		~ComponentPhysics();

		void ClearForces() { force = glm::vec3(0.0f); }
		void AddForce(glm::vec3 force) { this->force += force; }

		glm::vec3 Velocity() { return velocity; }
		glm::vec3 Force() { return force; }
		float InverseMass() { return inverseMass; }
		float RawMass() { return mass; }

		void SetMass(float newMass) {
			mass = newMass;
			UpdateInverseMass();
		}

		void SetVelocity(glm::vec3 newVelocity) { velocity = newVelocity; }

		bool Gravity() { return gravity; }
		void Gravity(bool gravity) { this->gravity = gravity; }

		ComponentTypes ComponentType() override { return COMPONENT_PHYSICS; }
		void Close() override;
	private:
		void UpdateInverseMass() { inverseMass = 1.0f / mass; }

		bool gravity;

		float inverseMass;
		float mass;
		glm::vec3 velocity;
		glm::vec3 force;
    };
}