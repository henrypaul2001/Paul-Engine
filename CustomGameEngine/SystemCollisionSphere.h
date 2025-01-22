#pragma once
#include "SystemCollision.h"
#include "ComponentTransform.h"
#include "ComponentCollisionSphere.h"
namespace Engine {
	class SystemCollisionSphere : public SystemCollision
	{
	public:
		SystemCollisionSphere(EntityManagerNew* ecs, CollisionManager* collisionManager) : SystemCollision(ecs, collisionManager) {}
		~SystemCollisionSphere() {}

		constexpr const char* SystemName() override { return "SYSTEM_COLLISION_SPHERE"; }

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentCollisionSphere& collider);
		void AfterAction();

	private:
		CollisionData Intersect(const unsigned int entityIDA, const unsigned int entityIDB, const ComponentTransform& transformA, const ComponentCollisionSphere& colliderA, const ComponentTransform& transformB, const ComponentCollisionSphere& colliderB) const;
	};
}