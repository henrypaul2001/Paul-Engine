#pragma once
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentCollision.h"
namespace Engine {
	class SystemCollision : public System
	{
	protected:
		void Collision(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2);
		virtual bool Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2) = 0;
		void DefaultCollisionResponse(Entity* entity1, Entity* entity2);
	public:
		virtual SystemTypes Name() override = 0;
		virtual void OnAction(Entity* entity) override = 0;
		virtual void AfterAction() override = 0;
	};
}