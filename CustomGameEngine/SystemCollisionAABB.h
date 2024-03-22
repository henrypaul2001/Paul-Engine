#pragma once
#include "System.h"
#include "SystemCollision.h"
#include "ComponentTransform.h"
#include "ComponentCollisionAABB.h"
namespace Engine{
	class SystemCollisionAABB : public SystemCollision
	{
	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_COLLISION_AABB);

		CollisionData Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2) override;
	public:
		SystemCollisionAABB(EntityManager* entityManager, CollisionManager* collisionManager);
		~SystemCollisionAABB();

		SystemTypes Name() override { return SYSTEM_COLLISION_AABB; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;
	};
}
