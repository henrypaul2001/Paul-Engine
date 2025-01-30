#include "SystemCollisionAABB.h"
#include <iostream>
namespace Engine {

	void SystemCollisionAABB::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentCollisionAABB& collider)
	{
		SCOPE_TIMER("SystemCollisionAABB::OnAction()");
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

	void SystemCollisionAABB::AfterAction()
	{
		SCOPE_TIMER("SystemCollisionAABB::AfterAction()");
		// Loop through all collision entities and clear EntitiesCheckedThisFrame
		active_ecs->View<ComponentCollisionAABB>().ForEach([](const unsigned int entityID, ComponentCollisionAABB& collider) {
			collider.ClearEntitiesCheckedThisFrame();
		});
	}

	CollisionData SystemCollisionAABB::Intersect(const unsigned int entityIDA, const unsigned int entityIDB, const ComponentTransform& transformA, const ComponentCollisionAABB& colliderA, const ComponentTransform& transformB, const ComponentCollisionAABB& colliderB) const
	{
		SCOPE_TIMER("SystemCollisionAABB::Intersect()");
		const AABBPoints bounds1 = colliderA.GetWorldSpaceBounds(transformA.GetWorldModelMatrix());
		const AABBPoints bounds2 = colliderB.GetWorldSpaceBounds(transformB.GetWorldModelMatrix());

		CollisionData collision;
		if (bounds1.minX <= bounds2.maxX && bounds1.maxX >= bounds2.minX && bounds1.minY <= bounds2.maxY && bounds1.maxY >= bounds2.minY && bounds1.minZ <= bounds2.maxZ && bounds1.maxZ >= bounds2.minZ) {
			const glm::vec3 faces[6] = {
				glm::vec3(-1.0f, 0.0f, 0.0f),
				glm::vec3(1.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, -1.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, -1.0f),
				glm::vec3(0.0f, 0.0f, 1.0f)
			};
			
			const float distances[6] = {
				(bounds2.maxX - bounds1.minX),
				(bounds1.maxX - bounds2.minX),
				(bounds2.maxY - bounds1.minY),
				(bounds1.maxY - bounds2.minY),
				(bounds2.maxZ - bounds1.minZ),
				(bounds1.maxZ - bounds2.minZ)
			};

			float penetration = FLT_MAX;
			glm::vec3 bestAxis;
			for (int i = 0; i < 6; i++) {
				if (distances[i] < penetration) {
					penetration = distances[i];
					bestAxis = faces[i];
				}
			}

			collision.isColliding = true;

			collision.AddContactPoint(glm::vec3(), glm::vec3(), bestAxis, penetration);

			collision.entityIDA = entityIDA;
			collision.entityIDB = entityIDB;

			GetContactPoints(collision);
		}
		else {
			collision.isColliding = false;
		}

		return collision;
	}
}