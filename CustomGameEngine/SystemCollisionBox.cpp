#include "SystemCollisionBox.h"
namespace Engine {
	void SystemCollisionBox::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentCollisionBox& collider)
	{
		// Loop through all other box entities for collision checks
		View<ComponentTransform, ComponentCollisionBox> aabbView = active_ecs->View<ComponentTransform, ComponentCollisionBox>();
		aabbView.ForEach([this, entityID, transform, &collider](const unsigned int entityIDB, ComponentTransform& transformB, ComponentCollisionBox& colliderB) {
			// Check if this entity has already checked for collisions with current entity in a previous run during this frame
			if (collider.HasEntityAlreadyBeenChecked(entityIDB) && entityIDB != entityID) {
				CollisionPreCheck(entityID, &collider, entityIDB, &colliderB);
				CollisionData collision = Intersect(entityID, entityIDB, transform, collider, transformB, colliderB);
				CollisionPostCheck(collision, entityID, &collider, entityIDB, &colliderB);
			}
		});
	}

	void SystemCollisionBox::AfterAction()
	{
		// Loop through all collision entities and clear EntitiesCheckedThisFrame
		active_ecs->View<ComponentCollisionBox>().ForEach([](const unsigned int entityID, ComponentCollisionBox& collider) {
			collider.ClearEntitiesCheckedThisFrame();
		});
	}

	CollisionData SystemCollisionBox::Intersect(const unsigned int entityIDA, const unsigned int entityIDB, const ComponentTransform& transformA, const ComponentCollisionBox& colliderA, const ComponentTransform& transformB, const ComponentCollisionBox& colliderB) const
	{
		CollisionData collision;
		if (colliderA.CheckBroadPhaseFirst() && colliderB.CheckBroadPhaseFirst()) {
			if (!BroadPhaseSphereSphere(transformA, colliderA, transformB, colliderB)) {
				//collision.objectA = transform->GetOwner();
				//collision.objectB = transform2->GetOwner();
				collision.isColliding = false;
				return collision;
			}
		}

		const std::vector<glm::vec3> axes = GetAllCollisionAxis(transformA, transformB);

		CollisionData bestCollision;
		bestCollision.AddContactPoint(glm::vec3(), glm::vec3(), glm::vec3(), -FLT_MAX);
		for (glm::vec3 axis : axes) {
			if (axis != glm::vec3(0.0f, 0.0f, 0.0f)) {
				//collision.objectA = transform->GetOwner();
				//collision.objectB = transform2->GetOwner();
				if (!CheckForCollisionOnAxis(axis, transformA, colliderA, transformB, colliderB, collision)) {
					collision.isColliding = false;
					return collision;
				}

				if (collision.contactPoints[0].penetration >= bestCollision.contactPoints[0].penetration) {
					bestCollision = collision;
				}
				collision = CollisionData();
			}
		}

		bestCollision.isColliding = true;

		GetContactPoints(bestCollision);

		return bestCollision;
	}
}