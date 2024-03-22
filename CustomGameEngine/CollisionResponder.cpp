#include "CollisionResponder.h"
#include "ComponentPhysics.h"
#include "ComponentTransform.h"
#include "ComponentCollision.h"
namespace Engine {
	CollisionResponder::CollisionResponder(CollisionManager* collisonManager)
	{
		this->collisionManager = collisonManager;
	}

	CollisionResponder::~CollisionResponder()
	{

	}

	void CollisionResponder::OnAction()
	{
		for (CollisionData collision : collisionManager->GetUnresolvedCollisions()) {
			Entity* objectA = collision.collidingObject;
			Entity* objectB = collision.otherCollidingObject;

			ComponentPhysics* physicsA = dynamic_cast<ComponentPhysics*>(objectA->GetComponent(COMPONENT_PHYSICS));
			ComponentPhysics* physicsB = dynamic_cast<ComponentPhysics*>(objectB->GetComponent(COMPONENT_PHYSICS));

			ComponentTransform* transformA = dynamic_cast<ComponentTransform*>(objectA->GetComponent(COMPONENT_TRANSFORM));
			ComponentTransform* transformB = dynamic_cast<ComponentTransform*>(objectB->GetComponent(COMPONENT_TRANSFORM));

			ComponentCollision* colliderA = dynamic_cast<ComponentCollision*>(objectA->GetComponent(COMPONENT_COLLISION_AABB));
			if (colliderA == nullptr) {
				colliderA = dynamic_cast<ComponentCollision*>(objectA->GetComponent(COMPONENT_COLLISION_SPHERE));
			}
			if (colliderA == nullptr) {
				colliderA = dynamic_cast<ComponentCollision*>(objectA->GetComponent(COMPONENT_COLLISION_BOX));
			}

			ComponentCollision* colliderB = dynamic_cast<ComponentCollision*>(objectB->GetComponent(COMPONENT_COLLISION_AABB));
			if (colliderB == nullptr) {
				colliderB = dynamic_cast<ComponentCollision*>(objectB->GetComponent(COMPONENT_COLLISION_SPHERE));
			}
			if (colliderB == nullptr) {
				colliderB = dynamic_cast<ComponentCollision*>(objectB->GetComponent(COMPONENT_COLLISION_BOX));
			}

			float totalMass = 0.0f;
			if (physicsA != nullptr) { totalMass += physicsA->InverseMass(); }
			if (physicsB != nullptr) { totalMass += physicsB->InverseMass(); }

			// Seperate objects using projection
			if (colliderA->IsMovedByCollisions()) {
				if (physicsA != nullptr) {
					transformA->SetPosition(transformA->GetWorldPosition() - (collision.collisionNormal * collision.collisionPenetration * (physicsA->InverseMass() / totalMass)));
				}
				else {
					transformA->SetPosition(transformA->GetWorldPosition() - (collision.collisionNormal * collision.collisionPenetration));
				}
			}
			if (colliderB->IsMovedByCollisions()) {
				if (physicsB != nullptr) {
					transformB->SetPosition(transformB->GetWorldPosition() - (collision.collisionNormal * collision.collisionPenetration * (physicsB->InverseMass() / totalMass)));
				}
				else {
					transformB->SetPosition(transformB->GetWorldPosition() - (collision.collisionNormal * collision.collisionPenetration));
				}
			}

			// Need to also change velocity. Currently, the ball just goes straight through floor after collision response due to it's high downward velocity being unchanged after the collision
		}
	}

	void CollisionResponder::AfterAction()
	{
		collisionManager->ClearUnresolvedCollisions();
	}
}