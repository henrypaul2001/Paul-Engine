#pragma once
#include "System.h"
namespace Engine {
	class SystemCollision : public System
	{
	public:
		virtual SystemTypes Name() override = 0;
		virtual void OnAction(Entity* entity) override = 0;
		virtual void AfterAction() override = 0;

		void DefaultCollisionResponse(Entity* entity1, Entity* entity2);
	};
}