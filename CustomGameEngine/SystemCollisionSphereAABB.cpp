#include "SystemCollisionSphereAABB.h"
namespace Engine {
	SystemCollisionSphereAABB::SystemCollisionSphereAABB(EntityManager* entityManager, CollisionManager* collisionManager) : SystemCollision(entityManager, collisionManager)
	{

	}

	SystemCollisionSphereAABB::~SystemCollisionSphereAABB()
	{

	}

	void SystemCollisionSphereAABB::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemCollisionSphereAABB::Run");
		System::Run(entityList);
	}

	void SystemCollisionSphereAABB::OnAction(Entity* entity)
	{
		if ((entity->Mask() & SPHERE_MASK) == SPHERE_MASK) {
			ComponentTransform* transform = entity->GetTransformComponent();
			ComponentCollisionSphere* collider = entity->GetSphereCollisionComponent();

			// Loop through all over entities to find another AABB collider entity
			for (Entity* e : entityManager->Entities()) {
				if ((e->Mask() & AABB_MASK) == AABB_MASK) {
					// Check if this entity has already checked for collisions with current entity in a previous run during this frame
					if (!collider->HasEntityAlreadyBeenChecked(e) && e != entity) {
						ComponentTransform* transform2 = e->GetTransformComponent();
						ComponentCollisionAABB* collider2 = e->GetAABBCollisionComponent();

						// Check for collision
						Collision(transform, collider, transform2, collider2);
					}
				}
			}
		}
	}

	void SystemCollisionSphereAABB::AfterAction()
	{
		// Loop through all collision entities and clear EntitiesCheckedThisFrame
		for (Entity* e : entityManager->Entities()) {
			if ((e->Mask() & SPHERE_MASK) == SPHERE_MASK) {
				dynamic_cast<ComponentCollisionSphere*>(e->GetComponent(COMPONENT_COLLISION_SPHERE))->ClearEntitiesCheckedThisFrame();
			}
			else if ((e->Mask() & AABB_MASK) == AABB_MASK) {
				dynamic_cast<ComponentCollisionAABB*>(e->GetComponent(COMPONENT_COLLISION_AABB))->ClearEntitiesCheckedThisFrame();
			}
		}
	}

	CollisionData SystemCollisionSphereAABB::Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		// collider = sphere
		// collider2 = AABB
		AABBPoints worldSpaceBounds = dynamic_cast<ComponentCollisionAABB*>(collider2)->GetWorldSpaceBounds(transform2->GetWorldModelMatrix());
		float scaledRadius = dynamic_cast<ComponentCollisionSphere*>(collider)->CollisionRadius() * transform->GetBiggestScaleFactor();

		// get closest point of AABB
		glm::vec3 closestPoint = glm::vec3();
		closestPoint.x = std::max(worldSpaceBounds.minX, std::min(transform->GetWorldPosition().x, worldSpaceBounds.maxX));
		closestPoint.y = std::max(worldSpaceBounds.minY, std::min(transform->GetWorldPosition().y, worldSpaceBounds.maxY));
		closestPoint.z = std::max(worldSpaceBounds.minZ, std::min(transform->GetWorldPosition().z, worldSpaceBounds.maxZ));

		float distance = glm::distance(closestPoint, transform->GetWorldPosition());

		glm::vec3 delta = transform->GetWorldPosition() - transform2->GetWorldPosition();
		glm::vec3 localPoint = delta - closestPoint;

		CollisionData collision;
		if (distance < scaledRadius) {
			collision.isColliding = true;

			float collisionPenetration = scaledRadius - distance;
			glm::vec3 collisionNormal = glm::normalize(transform->GetWorldPosition() - closestPoint);
			glm::vec3 localCollisionPoint = -collisionNormal * scaledRadius;
			glm::vec3 otherLocalCollisionPoint = glm::vec3();
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
