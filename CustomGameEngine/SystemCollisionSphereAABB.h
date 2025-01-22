#pragma once
#include "SystemCollision.h"
#include "ComponentTransform.h"
#include "ComponentCollisionAABB.h"
#include "ComponentCollisionSphere.h"
namespace Engine {
	class SystemCollisionSphereAABB : public SystemCollision
	{
	public:
		SystemCollisionSphereAABB(EntityManagerNew* ecs, CollisionManager* collisionManager) : SystemCollision(ecs, collisionManager) {}
		~SystemCollisionSphereAABB() {}

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentCollisionSphere& collider);
		void AfterAction();

	private:
		CollisionData Intersect(const unsigned int entityIDA, const unsigned int entityIDB, const ComponentTransform& transformA, const ComponentCollisionSphere& colliderA, const ComponentTransform& transformB, const ComponentCollisionAABB& colliderB) const;
	};
}