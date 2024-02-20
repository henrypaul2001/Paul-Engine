#pragma once
#include "Component.h"
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentVelocity.h"
namespace Engine 
{
	class SystemPhysics : public System
	{
	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_VELOCITY);
	public:
		SystemPhysics();
		~SystemPhysics();

		SystemTypes Name() override { return SYSTEM_PHYSICS; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;

		void Motion(ComponentTransform* transform, ComponentVelocity* velocity);
	};
}