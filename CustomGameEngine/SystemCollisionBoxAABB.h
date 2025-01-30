#pragma once
#include "SystemCollision.h"
namespace Engine {
    class SystemCollisionBoxAABB : public SystemCollision
    {
	public:
		SystemCollisionBoxAABB(EntityManager* ecs, CollisionManager* collisionManager) : SystemCollision(ecs, collisionManager) {}
		~SystemCollisionBoxAABB() {}

		constexpr const char* SystemName() override { return "SYSTEM_COLLISION_BOX_AABB"; }

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentCollisionBox& collider);
		void AfterAction();

	private:
		CollisionData Intersect(const unsigned int entityIDA, const unsigned int entityIDB, const ComponentTransform& transformA, const ComponentCollisionBox& colliderA, const ComponentTransform& transformB, const ComponentCollisionAABB& colliderB) const;
    };
}