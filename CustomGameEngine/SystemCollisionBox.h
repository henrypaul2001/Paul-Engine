#pragma once
#include "SystemCollision.h"
namespace Engine {
    class SystemCollisionBox : public SystemCollision
    {
	public:
		SystemCollisionBox(EntityManagerNew* ecs, CollisionManager* collisionManager) : SystemCollision(ecs, collisionManager) {}
		~SystemCollisionBox() {}

		constexpr const char* SystemName() override { return "SYSTEM_COLLISION_BOX"; }

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentCollisionBox& collider);
		void AfterAction();

	private:
		CollisionData Intersect(const unsigned int entityIDA, const unsigned int entityIDB, const ComponentTransform& transformA, const ComponentCollisionBox& colliderA, const ComponentTransform& transformB, const ComponentCollisionBox& colliderB) const;
    };
}