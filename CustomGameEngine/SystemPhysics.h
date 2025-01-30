#pragma once
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentPhysics.h"
namespace Engine 
{
	class SystemPhysics : public System
	{
	public:
		SystemPhysics(EntityManager* ecs, const float gravity = 9.8f, const glm::vec3& gravityAxis = glm::vec3(0.0f, 1.0f, 0.0f), const float airDensity = 1.225f) : System(ecs), gravity(gravity), gravityAxis(gravityAxis), airDensity(airDensity) {}
		~SystemPhysics() {}

		float airDensity; // kg/m3
		glm::vec3 gravityAxis;
		float gravity; // represented as acceleration m/s^2

		constexpr const char* SystemName() override { return "SYSTEM_PHYSICS"; }

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentPhysics& physics);
		void AfterAction();

	private:
		void Acceleration(ComponentTransform& transform, ComponentPhysics& physics);
	};
}