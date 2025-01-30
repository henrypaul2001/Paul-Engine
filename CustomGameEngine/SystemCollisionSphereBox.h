#pragma once
#include "SystemCollision.h"
namespace Engine {
    class SystemCollisionSphereBox : public SystemCollision
    {
	public:
		SystemCollisionSphereBox(EntityManager* ecs, CollisionManager* collisionManager) : SystemCollision(ecs, collisionManager) {}
		~SystemCollisionSphereBox() {}

		constexpr const char* SystemName() override { return "SYSTEM_COLLISION_SPHERE_BOX"; }

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentCollisionSphere& collider);
		void AfterAction();

	private:
		CollisionData Intersect(const unsigned int entityIDA, const unsigned int entityIDB, const ComponentTransform& transformA, const ComponentCollisionSphere& colliderA, const ComponentTransform& transformB, const ComponentCollisionBox& colliderB) const;
    };
}