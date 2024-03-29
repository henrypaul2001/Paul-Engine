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
			Entity* objectA = collision.objectA;
			Entity* objectB = collision.objectB;

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
		for (const ContactPoint& contact : collision.contactPoints) {
			if (colliderA->IsMovedByCollisions()) {
				if (physicsA != nullptr) {
					transformA->SetPosition(transformA->GetWorldPosition() + (contact.normal * contact.penetration) * (physicsA->InverseMass() / totalMass));
				}
				else {
					transformA->SetPosition(transformA->GetWorldPosition() + (contact.normal * contact.penetration));
				}
			}
			if (colliderB->IsMovedByCollisions()) {
				if (physicsB != nullptr) {
					transformB->SetPosition(transformB->GetWorldPosition() - (contact.normal * contact.penetration) * (physicsB->InverseMass() / totalMass));
				}
				else {
					transformB->SetPosition(transformB->GetWorldPosition() - (contact.normal * contact.penetration));
				}
			}
		}
	}

	void CollisionResponder::Impulse(ComponentTransform* transformA, ComponentPhysics* physicsA, ComponentCollision* colliderA, ComponentTransform* transformB, ComponentPhysics* physicsB, ComponentCollision* colliderB, float totalMass, CollisionData collision)
	{
		for (const ContactPoint& contact : collision.contactPoints) {
			glm::vec3 relativeA = contact.contactPointA;
			glm::vec3 relativeB = contact.contactPointB;

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

			float impulseForce = glm::dot(contactVelocity, contact.normal);

			glm::vec3 intertiaA = glm::vec3();
			glm::vec3 intertiaB = glm::vec3();
			if (physicsA != nullptr) { intertiaA = glm::cross(physicsA->InertiaTensor() * glm::cross(relativeA, contact.normal), relativeA); }
			if (physicsB != nullptr) { intertiaB = glm::cross(physicsB->InertiaTensor() * glm::cross(relativeB, contact.normal), relativeB); }

			float angularEffect = glm::dot(intertiaA + intertiaB, contact.normal);

			float elasticityA = 0.5f;
			float elasticityB = 0.5f;
			if (physicsA != nullptr) { elasticityA = physicsA->Elasticity(); }
			if (physicsB != nullptr) { elasticityB = physicsB->Elasticity(); }

			float coefficient = elasticityA * elasticityB;

			float J = (-(1.0f + coefficient) * impulseForce) / (totalMass + angularEffect);
			glm::vec3 fullImpulse = contact.normal * J;

			if (physicsA != nullptr && colliderA->IsMovedByCollisions()) {
				physicsA->ApplyLinearImpulse(-fullImpulse);
				physicsA->ApplyAngularImpulse(glm::cross(relativeA, -fullImpulse));
			}

			if (physicsB != nullptr && colliderB->IsMovedByCollisions()) {
				physicsB->ApplyLinearImpulse(fullImpulse);
				physicsB->ApplyAngularImpulse(glm::cross(relativeB, fullImpulse));
			}
		}
	}
}