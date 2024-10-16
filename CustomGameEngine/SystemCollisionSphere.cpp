#include "SystemCollisionSphere.h"
namespace Engine {
	SystemCollisionSphere::SystemCollisionSphere(EntityManager* entityManager, CollisionManager* collisionManager) : SystemCollision(entityManager, collisionManager)
	{
	}

	SystemCollisionSphere::~SystemCollisionSphere()
	{

	}

	void SystemCollisionSphere::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemCollisionSphere::Run");
		System::Run(entityList);
	}

	void SystemCollisionSphere::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
			ComponentTransform* transform = entity->GetTransformComponent();
			ComponentCollisionSphere* collider = entity->GetSphereCollisionComponent();

			// Loop through all over entities to find another AABB collider entity
			for (Entity* e : entityManager->Entities()) {
				if ((e->Mask() & MASK) == MASK) {
					// Check if this entity has already checked for collisions with current entity in a previous run during this frame
					if (!collider->HasEntityAlreadyBeenChecked(e) && e != entity) {
						ComponentTransform* transform2 = e->GetTransformComponent();
						ComponentCollisionSphere* collider2 = e->GetSphereCollisionComponent();

						// Check for collision
						Collision(transform, collider, transform2, collider2);
					}
				}
			}
		}
	}

	void SystemCollisionSphere::AfterAction()
	{
		// Loop through all collision entities and clear EntitiesCheckedThisFrame
		for (Entity* e : entityManager->Entities()) {
			if ((e->Mask() & MASK) == MASK) {
				dynamic_cast<ComponentCollisionSphere*>(e->GetComponent(COMPONENT_COLLISION_SPHERE))->ClearEntitiesCheckedThisFrame();
			}
		}
	}

	CollisionData SystemCollisionSphere::Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		float scaledRadius1 = dynamic_cast<ComponentCollisionSphere*>(collider)->CollisionRadius() * transform->GetBiggestScaleFactor();
		float scaledRadius2 = dynamic_cast<ComponentCollisionSphere*>(collider2)->CollisionRadius() * transform2->GetBiggestScaleFactor();

		float distanceSqr = glm::distance2(transform->GetWorldPosition(), transform2->GetWorldPosition());

		float combinedRadius = scaledRadius1 + scaledRadius2;
		float combinedRadiusSqr = combinedRadius * combinedRadius;

		CollisionData collision;
		if (distanceSqr < combinedRadiusSqr) {
			collision.isColliding = true;
			float distance = glm::sqrt(distanceSqr);
			float collisionPenetration = combinedRadius- distance;
			glm::vec3 collisionNormal = -glm::normalize(transform2->GetWorldPosition() - transform->GetWorldPosition());
			glm::vec3 localCollisionPoint = collisionNormal * dynamic_cast<ComponentCollisionSphere*>(collider)->CollisionRadius();
			glm::vec3 otherLocalCollisionPoint = -collisionNormal * dynamic_cast<ComponentCollisionSphere*>(collider2)->CollisionRadius();
			collision.AddContactPoint(localCollisionPoint, otherLocalCollisionPoint, collisionNormal, collisionPenetration);

			collision.objectA = transform->GetOwner();
			collision.objectB = transform2->GetOwner();
		}
		else {
			collision.isColliding = false;
		}

		return collision;
	}
}