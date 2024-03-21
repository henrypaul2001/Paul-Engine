#pragma once
#include "Component.h"
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentVelocity.h"
#include "ComponentPhysics.h"
namespace Engine 
{
	class SystemPhysics : public System
	{
	public:
		SystemPhysics();
		~SystemPhysics();

		glm::vec3 GravityAxis() { return gravityAxis; }
		float Gravity() { return gravity; }
		void Gravity(float newGravity) { gravity = newGravity; }
		void Gravity(glm::vec3 gravityAxis) { this->gravityAxis = gravityAxis; }
		void Gravity(float gravityScale, glm::vec3 gravityAxis) {
			gravity = gravityScale;
			this->gravityAxis = gravityAxis;
		}

		SystemTypes Name() override { return SYSTEM_PHYSICS; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;

	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_PHYSICS);

		glm::vec3 gravityAxis;
		float gravity; // represented as acceleration m/s^2

		void Acceleration(ComponentPhysics* physics);

		void Physics(ComponentTransform* transform, ComponentPhysics* physics);
	};
}