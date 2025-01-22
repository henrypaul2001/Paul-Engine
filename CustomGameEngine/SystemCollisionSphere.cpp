#include "SystemCollisionSphere.h"
#include <glm/gtx/norm.hpp>
namespace Engine {
	void SystemCollisionSphere::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentCollisionSphere& collider)
	{
		// Loop through all other sphere entities for collision checks
		View<ComponentTransform, ComponentCollisionSphere> aabbView = active_ecs->View<ComponentTransform, ComponentCollisionSphere>();
		aabbView.ForEach([this, entityID, transform, &collider](const unsigned int entityIDB, ComponentTransform& transformB, ComponentCollisionSphere& colliderB) {
			// Check if this entity has already checked for collisions with current entity in a previous run during this frame
			if (collider.HasEntityAlreadyBeenChecked(entityIDB) && entityIDB != entityID) {
				CollisionPreCheck(entityID, &collider, entityIDB, &colliderB);
				CollisionData collision = Intersect(entityID, entityIDB, transform, collider, transformB, colliderB);
				CollisionPostCheck(collision, entityID, &collider, entityIDB, &colliderB);
			}
		});
	}

	void SystemCollisionSphere::AfterAction()
	{
		// Loop through all collision entities and clear EntitiesCheckedThisFrame
		active_ecs->View<ComponentCollisionSphere>().ForEach([](const unsigned int entityID, ComponentCollisionSphere& collider) {
			collider.ClearEntitiesCheckedThisFrame();
		});
	}

	CollisionData SystemCollisionSphere::Intersect(const unsigned int entityIDA, const unsigned int entityIDB, const ComponentTransform& transformA, const ComponentCollisionSphere& colliderA, const ComponentTransform& transformB, const ComponentCollisionSphere& colliderB) const
	{
		const float scaledRadius1 = colliderA.CollisionRadius() * transformA.GetBiggestScaleFactor();
		const float scaledRadius2 = colliderB.CollisionRadius() * transformB.GetBiggestScaleFactor();

		const float distanceSqr = glm::distance2(transformA.GetWorldPosition(), transformB.GetWorldPosition());

		const float combinedRadius = scaledRadius1 + scaledRadius2;
		const float combinedRadiusSqr = combinedRadius * combinedRadius;

		CollisionData collision;
		if (distanceSqr < combinedRadiusSqr) {
			collision.isColliding = true;
			const float distance = glm::sqrt(distanceSqr);
			const float collisionPenetration = combinedRadius - distance;
			const glm::vec3 collisionNormal = -glm::normalize(transformB.GetWorldPosition() - transformA.GetWorldPosition());
			const glm::vec3 localCollisionPoint = collisionNormal * colliderA.CollisionRadius();
			const glm::vec3 otherLocalCollisionPoint = -collisionNormal * colliderB.CollisionRadius();
			collision.AddContactPoint(localCollisionPoint, otherLocalCollisionPoint, collisionNormal, collisionPenetration);

			//collision.objectA = transform->GetOwner();
			//collision.objectB = transform2->GetOwner();
		}
		else {
			collision.isColliding = false;
		}

		return collision;
	}
}