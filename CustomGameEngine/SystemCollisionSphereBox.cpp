#include "SystemCollisionSphereBox.h"
namespace Engine {
	void SystemCollisionSphereBox::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentCollisionSphere& collider)
	{
		// Loop through all other box entities for collision checks
		View<ComponentTransform, ComponentCollisionBox> boxView = active_ecs->View<ComponentTransform, ComponentCollisionBox>();
		boxView.ForEach([this, entityID, transform, &collider](const unsigned int entityIDB, ComponentTransform& transformB, ComponentCollisionBox& colliderB) {
			// Check if this entity has already checked for collisions with current entity in a previous run during this frame
			if (collider.HasEntityAlreadyBeenChecked(entityIDB) && entityIDB != entityID) {
				CollisionPreCheck(entityID, &collider, entityIDB, &colliderB);
				CollisionData collision = Intersect(entityID, entityIDB, transform, collider, transformB, colliderB);
				CollisionPostCheck(collision, entityID, &collider, entityIDB, &colliderB);
			}
		});
	}

	void SystemCollisionSphereBox::AfterAction()
	{
		// Loop through all collision entities and clear EntitiesCheckedThisFrame
		active_ecs->View<ComponentCollisionSphere>().ForEach([](const unsigned int entityID, ComponentCollisionSphere& collider) {
			collider.ClearEntitiesCheckedThisFrame();
		});
		active_ecs->View<ComponentCollisionBox>().ForEach([](const unsigned int entityID, ComponentCollisionBox& collider) {
			collider.ClearEntitiesCheckedThisFrame();
		});
	}

	CollisionData SystemCollisionSphereBox::Intersect(const unsigned int entityIDA, const unsigned int entityIDB, const ComponentTransform& transformA, const ComponentCollisionSphere& colliderA, const ComponentTransform& transformB, const ComponentCollisionBox& colliderB) const
	{
		// Transform sphere into oriented box's local space
		const glm::vec3& worldSpaceSpherePosition = transformA.GetWorldPosition();
		const glm::mat4& boxTransform = transformB.GetWorldModelMatrix();

		const glm::vec3 transformedSherePosition = glm::vec3(glm::inverse(boxTransform) * glm::vec4(worldSpaceSpherePosition, 1.0f));

		// Find closest point on box to sphere position
		const BoxExtents& localMinMax = colliderB.GetLocalPoints();
		glm::vec3 closestPoint = glm::vec3();
		closestPoint.x = std::max(localMinMax.minX, std::min(transformedSherePosition.x, localMinMax.maxX));
		closestPoint.y = std::max(localMinMax.minY, std::min(transformedSherePosition.y, localMinMax.maxY));
		closestPoint.z = std::max(localMinMax.minZ, std::min(transformedSherePosition.z, localMinMax.maxZ));

		// Transform closest point into world space
		const glm::vec3 closestPointWorldSpace = glm::vec3(boxTransform * glm::vec4(closestPoint, 1.0f));

		const float distance = glm::distance(closestPointWorldSpace, worldSpaceSpherePosition);
		const float scaledRadius = colliderA.CollisionRadius() * transformA.GetBiggestScaleFactor();

		CollisionData collision;
		if (distance < scaledRadius) {
			collision.isColliding = true;

			const float collisionPenetration = scaledRadius - distance;
			const glm::vec3 collisionNormal = -glm::normalize(worldSpaceSpherePosition - closestPointWorldSpace);
			const glm::vec3 localCollisionPoint = closestPoint;
			const glm::vec3 otherLocalCollisionPoint = -collisionNormal * scaledRadius;
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