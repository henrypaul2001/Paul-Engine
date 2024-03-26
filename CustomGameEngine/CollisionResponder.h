#pragma once
#include "CollisionManager.h"
#include "ComponentPhysics.h"
#include "ComponentTransform.h"
#include "ComponentCollision.h"
namespace Engine {
	class CollisionResponder
	{
	public:
		CollisionResponder(CollisionManager* collisonManager);
		~CollisionResponder();

		void OnAction();
		void AfterAction();
	private:
		void Separate(ComponentTransform* transformA, ComponentPhysics* physicsA, ComponentCollision* colliderA, ComponentTransform* transformB, ComponentPhysics* physicsB, ComponentCollision* colliderB, float totalMass, CollisionData collision);
		void Impulse(ComponentTransform* transformA, ComponentPhysics* physicsA, ComponentCollision* colliderA, ComponentTransform* transformB, ComponentPhysics* physicsB, ComponentCollision* colliderB, float totalMass, CollisionData collision);
		CollisionManager* collisionManager;
	};
}