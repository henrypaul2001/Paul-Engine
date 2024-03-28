#include "CollisionResponder.h"
#include "Scene.h"
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
			Separate(transformA, physicsA, colliderA, transformB, physicsB, colliderB, totalMass, collision);

			// Update velocity of each physics component if they exist
			Impulse(transformA, physicsA, colliderA, transformB, physicsB, colliderB, totalMass, collision);
		}
	}

	void CollisionResponder::AfterAction()
	{
		collisionManager->ClearUnresolvedCollisions();
	}

	void CollisionResponder::Separate(ComponentTransform* transformA, ComponentPhysics* physicsA, ComponentCollision* colliderA, ComponentTransform* transformB, ComponentPhysics* physicsB, ComponentCollision* colliderB, float totalMass, CollisionData collision)
	{
		if (colliderA->IsMovedByCollisions()) {
			if (physicsA != nullptr) {
				transformA->SetPosition(transformA->GetWorldPosition() + (collision.collisionNormal * collision.collisionPenetration) * (physicsA->InverseMass() / totalMass));
			}
			else {
				transformA->SetPosition(transformA->GetWorldPosition() + (collision.collisionNormal * collision.collisionPenetration));
			}
		}
		if (colliderB->IsMovedByCollisions()) {
			if (physicsB != nullptr) {
				transformB->SetPosition(transformB->GetWorldPosition() - (collision.collisionNormal * collision.collisionPenetration) * (physicsB->InverseMass() / totalMass));
			}
			else {
				transformB->SetPosition(transformB->GetWorldPosition() - (collision.collisionNormal * collision.collisionPenetration));
			}
		}
	}

	void CollisionResponder::Impulse(ComponentTransform* transformA, ComponentPhysics* physicsA, ComponentCollision* colliderA, ComponentTransform* transformB, ComponentPhysics* physicsB, ComponentCollision* colliderB, float totalMass, CollisionData collision)
	{
		//glm::vec3 relativeA = collision.otherLocalCollisionPoint - transformA->GetWorldPosition();
		//glm::vec3 relativeB = collision.localCollisionPoint - transformB->GetWorldPosition();

		glm::vec3 relativeA = collision.localCollisionPoint;
		glm::vec3 relativeB = collision.otherLocalCollisionPoint;

		glm::vec3 angularVelocityA = glm::vec3();
		glm::vec3 velocityA = glm::vec3();

		glm::vec3 angularVelocityB = glm::vec3();
		glm::vec3 velocityB = glm::vec3();

		if (physicsA != nullptr) { 
			angularVelocityA = glm::cross(physicsA->AngularVelocity(), relativeA);
			velocityA = physicsA->Velocity() + angularVelocityA;
		}
		if (physicsB != nullptr) { 
			angularVelocityB = glm::cross(physicsB->AngularVelocity(), relativeB);
			velocityB = physicsB->Velocity() + angularVelocityB;
		}

		glm::vec3 contactVelocity = velocityB - velocityA;

		float impulseForce = glm::dot(contactVelocity, collision.collisionNormal);

		glm::vec3 intertiaA = glm::vec3();
		glm::vec3 intertiaB = glm::vec3();
		if (physicsA != nullptr) { intertiaA = glm::cross(physicsA->InertiaTensor() * glm::cross(relativeA, collision.collisionNormal), relativeA); }
		if (physicsB != nullptr) { intertiaB = glm::cross(physicsB->InertiaTensor() * glm::cross(relativeB, collision.collisionNormal), relativeB); }

		float angularEffect = glm::dot(intertiaA + intertiaB, collision.collisionNormal);

		float elasticityA = 0.5f;
		float elasticityB = 0.5f;
		if (physicsA != nullptr) { elasticityA = physicsA->Elasticity(); }
		if (physicsB != nullptr) { elasticityB = physicsB->Elasticity(); }

		//float coefficient = 0.66f; // hard coded value representing energy lost on collision, will later be determined by physical properties in physics component
		float coefficient = elasticityA * elasticityB;

		float J = (-(1.0f + coefficient) * impulseForce) / (totalMass + angularEffect);
		glm::vec3 fullImpulse = collision.collisionNormal * J;

		if (physicsA != nullptr && colliderA->IsMovedByCollisions()) {
			//physicsA->AddForce(-fullImpulse, relativeA);
			physicsA->ApplyLinearImpulse(-fullImpulse);
			physicsA->ApplyAngularImpulse(glm::cross(relativeA, -fullImpulse));
		}

		if (physicsB != nullptr && colliderB->IsMovedByCollisions()) {
			//physicsB->AddForce(fullImpulse, relativeB);
			physicsB->ApplyLinearImpulse(fullImpulse);
			physicsB->ApplyAngularImpulse(glm::cross(relativeB, fullImpulse));
		}
	}
}