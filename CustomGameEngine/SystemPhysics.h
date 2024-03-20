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

		float Gravity() { return gravity; }
		void Gravity(float newGravity) { gravity = newGravity; }

		SystemTypes Name() override { return SYSTEM_PHYSICS; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;

	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_PHYSICS);

		float gravity; // represented as acceleration m/s^2

		void Acceleration(ComponentPhysics* physics);

		void Physics(ComponentTransform* transform, ComponentPhysics* physics);
	};
}