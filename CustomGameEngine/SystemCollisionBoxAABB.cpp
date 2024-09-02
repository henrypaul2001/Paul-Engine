#include "SystemCollisionBoxAABB.h"
namespace Engine {
	SystemCollisionBoxAABB::SystemCollisionBoxAABB(EntityManager* entityManager, CollisionManager* collisionManager) : SystemCollision(entityManager, collisionManager)
	{

	}

	SystemCollisionBoxAABB::~SystemCollisionBoxAABB()
	{

	}

	void SystemCollisionBoxAABB::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemCollisionBoxAABB::Run");
		System::Run(entityList);
	}

	void SystemCollisionBoxAABB::OnAction(Entity* entity)
	{
		if ((entity->Mask() & BOX_MASK) == BOX_MASK) {
			ComponentTransform* transform = entity->GetTransformComponent();
			ComponentCollisionBox* collider = entity->GetBoxCollisionComponent();

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

	void SystemCollisionBoxAABB::AfterAction()
	{
		// Loop through all collision entities and clear EntitiesCheckedThisFrame
		for (Entity* e : entityManager->Entities()) {
			if ((e->Mask() & BOX_MASK) == BOX_MASK) {
				dynamic_cast<ComponentCollisionBox*>(e->GetComponent(COMPONENT_COLLISION_BOX))->ClearEntitiesCheckedThisFrame();
			}
			else if ((e->Mask() & AABB_MASK) == AABB_MASK) {
				dynamic_cast<ComponentCollisionAABB*>(e->GetComponent(COMPONENT_COLLISION_AABB))->ClearEntitiesCheckedThisFrame();
			}
		}
	}

	CollisionData SystemCollisionBoxAABB::Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		CollisionData collision;
		if (dynamic_cast<ComponentCollisionBox*>(collider)->CheckBroadPhaseFirst()) {
			if (!BroadPhaseSphereSphere(transform, collider, transform2, collider2)) {
				collision.objectA = transform->GetOwner();
				collision.objectB = transform2->GetOwner();
				collision.isColliding = false;
				return collision;
			}
		}

		std::vector<glm::vec3> axes = GetAllCollisionAxis(transform, transform2);

		CollisionData bestCollision;
		bestCollision.AddContactPoint(glm::vec3(), glm::vec3(), glm::vec3(), -FLT_MAX);
		float penetration = -FLT_MAX;
		for (glm::vec3 axis : axes) {
			collision.objectA = transform->GetOwner();
			collision.objectB = transform2->GetOwner();
			if (!CheckForCollisionOnAxis(axis, transform, dynamic_cast<ComponentCollisionBox*>(collider), transform2, dynamic_cast<ComponentCollisionAABB*>(collider2), collision)) {
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