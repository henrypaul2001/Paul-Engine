#include "SystemCollisionBoxAABB.h"
namespace Engine {
	void SystemCollisionBoxAABB::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentCollisionBox& collider)
	{
		SCOPE_TIMER("SystemCollisionBoxAABB::OnAction()");
		// Loop through all AABB entities for collision checks
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

	void SystemCollisionBoxAABB::AfterAction()
	{
		SCOPE_TIMER("SystemCollisionBoxAABB::AfterAction()");
		// Loop through all collision entities and clear EntitiesCheckedThisFrame
		active_ecs->View<ComponentCollisionBox>().ForEach([](const unsigned int entityID, ComponentCollisionBox& collider) {
			collider.ClearEntitiesCheckedThisFrame();
		});
		active_ecs->View<ComponentCollisionAABB>().ForEach([](const unsigned int entityID, ComponentCollisionAABB& collider) {
			collider.ClearEntitiesCheckedThisFrame();
		});
	}

	CollisionData SystemCollisionBoxAABB::Intersect(const unsigned int entityIDA, const unsigned int entityIDB, const ComponentTransform& transformA, const ComponentCollisionBox& colliderA, const ComponentTransform& transformB, const ComponentCollisionAABB& colliderB) const
	{
		SCOPE_TIMER("SystemCollisionBoxAABB::Intersect()");
		CollisionData collision;
		if (colliderA.CheckBroadPhaseFirst()) {
			if (!BroadPhaseSphereSphere(transformA, colliderA, transformB, colliderB)) {
				collision.entityIDA = entityIDA;
				collision.entityIDB = entityIDB;
				collision.isColliding = false;
				return collision;
			}
		}

		std::vector<glm::vec3> axes = GetAllCollisionAxis(transformA, transformB);

		CollisionData bestCollision;
		bestCollision.AddContactPoint(glm::vec3(), glm::vec3(), glm::vec3(), -FLT_MAX);
		float penetration = -FLT_MAX;
		for (glm::vec3 axis : axes) {
			collision.entityIDA = entityIDA;
			collision.entityIDB = entityIDB;
			if (!CheckForCollisionOnAxis(axis, transformA, colliderA, transformB, colliderB, collision)) {
				collision.isColliding = false;
				return collision;
			}

			if (collision.contactPoints[0].penetration >= bestCollision.contactPoints[0].penetration) {
				bestCollision = collision;
			}
			collision = CollisionData();
		}

		bestCollision.isColliding = true;

		GetContactPoints(bestCollision);

		return bestCollision;
	}
}