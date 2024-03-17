#pragma once
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentCollisionAABB.h"
#include "EntityManager.h"
namespace Engine{
	class SystemCollisionAABB : public System
	{
	private:
		EntityManager* entityManager;
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_COLLISION_AABB);
	public:
		SystemCollisionAABB(EntityManager* entityManager);
		~SystemCollisionAABB();

		SystemTypes Name() override { return SYSTEM_COLLISION_AABB; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;

		void AABBCollision(ComponentTransform* transform, ComponentCollisionAABB* collider, ComponentTransform* transform2, ComponentCollisionAABB* collider2);
	};
}
