#include "SystemCollisionBox.h"
namespace Engine {
	SystemCollisionBox::SystemCollisionBox(EntityManager* entityManager, CollisionManager* collisionManager) : SystemCollision(entityManager, collisionManager)
	{

	}

	SystemCollisionBox::~SystemCollisionBox()
	{

	}

	void SystemCollisionBox::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemCollisionBox::Run");
		System::Run(entityList);
	}

	void SystemCollisionBox::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
			ComponentTransform* transform = entity->GetTransformComponent();
			ComponentCollisionBox* collider = entity->GetBoxCollisionComponent();

			// Loop through all over entities to find another box collider entity
			for (Entity* e : entityManager->Entities()) {
				if ((e->Mask() & MASK) == MASK) {
					// Check if this entity has already checked for collisions with current entity in a previous run during this frame
					if (!collider->HasEntityAlreadyBeenChecked(e) && e != entity) {
						ComponentTransform* transform2 = e->GetTransformComponent();
						ComponentCollisionBox* collider2 = e->GetBoxCollisionComponent();

						// Check for collision
						Collision(transform, collider, transform2, collider2);
					}
				}
			}
		}
	}

	void SystemCollisionBox::AfterAction()
	{
		// Loop through all collision entities and clear EntitiesCheckedThisFrame
		for (Entity* e : entityManager->Entities()) {
			if ((e->Mask() & MASK) == MASK) {
				dynamic_cast<ComponentCollisionBox*>(e->GetComponent(COMPONENT_COLLISION_BOX))->ClearEntitiesCheckedThisFrame();
			}
		}
	}

	CollisionData SystemCollisionBox::Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		CollisionData collision;
		if (dynamic_cast<ComponentCollisionBox*>(collider)->CheckBroadPhaseFirst() && dynamic_cast<ComponentCollisionBox*>(collider2)->CheckBroadPhaseFirst()) {
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
		for (glm::vec3 axis : axes) {
			if (axis != glm::vec3(0.0f, 0.0f, 0.0f)) {
				collision.objectA = transform->GetOwner();
				collision.objectB = transform2->GetOwner();
				if (!CheckForCollisionOnAxis(axis, transform, dynamic_cast<ComponentCollisionBox*>(collider), transform2, dynamic_cast<ComponentCollisionBox*>(collider2), collision)) {
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