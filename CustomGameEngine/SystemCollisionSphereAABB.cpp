#include "SystemCollisionSphereAABB.h"
namespace Engine {
	void SystemCollisionSphereAABB::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentCollisionSphere& collider)
	{
		// Loop through all other AABB entities for collision checks
		View<ComponentTransform, ComponentCollisionAABB> aabbView = active_ecs->View<ComponentTransform, ComponentCollisionAABB>();
		aabbView.ForEach([this, entityID, transform, &collider](const unsigned int entityIDB, ComponentTransform& transformB, ComponentCollisionAABB& colliderB) {
			// Check if this entity has already checked for collisions with current entity in a previous run during this frame
			if (!collider.HasEntityAlreadyBeenChecked(entityIDB) && entityIDB != entityID) {
				CollisionPreCheck(entityID, &collider, entityIDB, &colliderB);
				CollisionData collision = Intersect(entityID, entityIDB, transform, collider, transformB, colliderB);
				CollisionPostCheck(collision, entityID, &collider, entityIDB, &colliderB);
			}
		});
	}

	void SystemCollisionSphereAABB::AfterAction()
	{
		// Loop through all collision entities and clear EntitiesCheckedThisFrame
		active_ecs->View<ComponentCollisionSphere>().ForEach([](const unsigned int entityID, ComponentCollisionSphere& collider) {
			collider.ClearEntitiesCheckedThisFrame();
		});
		active_ecs->View<ComponentCollisionAABB>().ForEach([](const unsigned int entityID, ComponentCollisionAABB& collider) {
			collider.ClearEntitiesCheckedThisFrame();
		});
	}

	CollisionData SystemCollisionSphereAABB::Intersect(const unsigned int entityIDA, const unsigned int entityIDB, const ComponentTransform& transformA, const ComponentCollisionSphere& colliderA, const ComponentTransform& transformB, const ComponentCollisionAABB& colliderB) const
	{
		// collider = sphere
		// collider2 = AABB
		const AABBPoints worldSpaceBounds = colliderB.GetWorldSpaceBounds(transformB.GetWorldModelMatrix());
		const float scaledRadius = colliderA.CollisionRadius() * transformA.GetBiggestScaleFactor();

		const glm::vec3& worldPosA = transformA.GetWorldPosition();
		const glm::vec3& worldPosB = transformB.GetWorldPosition();

		// get closest point of AABB
		glm::vec3 closestPoint = glm::vec3();
		closestPoint.x = std::max(worldSpaceBounds.minX, std::min(worldPosA.x, worldSpaceBounds.maxX));
		closestPoint.y = std::max(worldSpaceBounds.minY, std::min(worldPosA.y, worldSpaceBounds.maxY));
		closestPoint.z = std::max(worldSpaceBounds.minZ, std::min(worldPosA.z, worldSpaceBounds.maxZ));

		const float distance = glm::distance(closestPoint, worldPosA);

		const glm::vec3 delta = worldPosA - worldPosB;
		const glm::vec3 localPoint = delta - closestPoint;

		CollisionData collision;
		if (distance < scaledRadius) {
			collision.isColliding = true;

			const float collisionPenetration = scaledRadius - distance;
			const glm::vec3 collisionNormal = glm::normalize(worldPosA - closestPoint);
			const glm::vec3 localCollisionPoint = -collisionNormal * scaledRadius;
			const glm::vec3 otherLocalCollisionPoint = glm::vec3();
			collision.AddContactPoint(localCollisionPoint, otherLocalCollisionPoint, collisionNormal, collisionPenetration);

			collision.entityIDA = entityIDA;
			collision.entityIDB = entityIDB;
		}
		else {
			collision.isColliding = false;
		}

		return collision;
	}
}
