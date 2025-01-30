#pragma once
#include "EntityManager.h"
#include "CollisionManager.h"
#include "ComponentPhysics.h"
#include "ComponentTransform.h"
#include "ComponentCollisionSphere.h"
#include "ComponentCollisionBox.h"
#include "ComponentCollisionAABB.h"

namespace Engine {
	class CollisionResolver
	{
	public:
		CollisionResolver(CollisionManager* collisionManager) : collisionManager(collisionManager) {}
		~CollisionResolver() {}

		void Run(EntityManager& ecs);

	private:
		void Separate(ComponentTransform* transformA, ComponentPhysics* physicsA, ComponentCollision* colliderA, ComponentTransform* transformB, ComponentPhysics* physicsB, ComponentCollision* colliderB, const float totalMass, const CollisionData& collision) const;
		void Impulse(ComponentTransform* transformA, ComponentPhysics* physicsA, ComponentCollision* colliderA, ComponentTransform* transformB, ComponentPhysics* physicsB, ComponentCollision* colliderB, const float totalMass, const CollisionData& collision) const;
		
		//void PresolveContactPoint(ContactPoint& contact, Entity* objectA, Entity* objectB, int numContacts);
		//void SolveContactPoint(ContactPoint& contact, Entity* objectA, Entity* objectB, int numContacts);
		
		CollisionManager* collisionManager;
	};
}