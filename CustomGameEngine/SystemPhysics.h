#pragma once
#include "Component.h"
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentVelocity.h"
namespace Engine 
{
	class SystemPhysics : System
	{
	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_VELOCITY);
	public:
		SystemPhysics();
		~SystemPhysics();

		std::string Name() override { return "SystemPhysics"; }
		void OnAction(Entity* entity) override;

		void Motion(ComponentTransform* transform, ComponentVelocity* velocity);
	};
}