#pragma once
#include "SystemCollision.h"
#include "ComponentTransform.h"
#include "ComponentCollisionAABB.h"
namespace Engine{
	class SystemCollisionAABB : public SystemCollision
	{
	public:
		SystemCollisionAABB(EntityManager* ecs, CollisionManager* collisionManager) : SystemCollision(ecs, collisionManager) {}
		~SystemCollisionAABB() {}

		constexpr const char* SystemName() override { return "SYSTEM_COLLISION_AABB"; }

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentCollisionAABB& collider);
		void AfterAction();

	private:
		CollisionData Intersect(const unsigned int entityIDA, const unsigned int entityIDB, const ComponentTransform& transformA, const ComponentCollisionAABB& colliderA, const ComponentTransform& transformB, const ComponentCollisionAABB& colliderB) const;
	};
}
