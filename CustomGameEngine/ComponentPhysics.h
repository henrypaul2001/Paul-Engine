#pragma once
#include "Component.h"
#include <glm/ext/vector_float3.hpp>
namespace Engine {
    class ComponentPhysics : public Component
    {
	public:
		ComponentPhysics(float mass);
		~ComponentPhysics();

		void ClearForces() { force = glm::vec3(0.0f); }
		void AddForce(glm::vec3 force) { this->force += force; }

		glm::vec3 Velocity() { return velocity; }
		glm::vec3 Force() { return force; }
		float InverseMass() { inverseMass; }
		float RawMass() { return mass; }

		ComponentTypes ComponentType() override { return COMPONENT_PHYSICS; }
		void Close() override;
	private:
		float inverseMass;
		float mass;
		glm::vec3 velocity;
		glm::vec3 force;
    };
}