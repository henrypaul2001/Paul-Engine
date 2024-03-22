#include "SystemCollisionBoxAABB.h"
namespace Engine {
	SystemCollisionBoxAABB::SystemCollisionBoxAABB(EntityManager* entityManager, CollisionManager* collisionManager) : SystemCollision(entityManager, collisionManager)
	{

	}

	SystemCollisionBoxAABB::~SystemCollisionBoxAABB()
	{

	}

	void SystemCollisionBoxAABB::OnAction(Entity* entity)
	{
		if ((entity->Mask() & BOX_MASK) == BOX_MASK) {
			std::vector<Component*> components = entity->Components();

			ComponentTransform* transform = nullptr;
			for (Component* c : components) {
				transform = dynamic_cast<ComponentTransform*>(c);
				if (transform != nullptr) {
					break;
				}
			}

			ComponentCollisionBox* collider = nullptr;
			for (Component* c : components) {
				collider = dynamic_cast<ComponentCollisionBox*>(c);
				if (collider != nullptr) {
					break;
				}
			}

			// Loop through all over entities to find another AABB collider entity
			for (Entity* e : entityManager->Entities()) {
				if ((e->Mask() & AABB_MASK) == AABB_MASK) {
					// Check if this entity has already checked for collisions with current entity in a previous run during this frame
					if (!collider->HasEntityAlreadyBeenChecked(e) && e != entity) {

						std::vector<Component*> components2 = e->Components();

						ComponentTransform* transform2 = nullptr;
						for (Component* c : components2) {
							transform2 = dynamic_cast<ComponentTransform*>(c);
							if (transform2 != nullptr) {
								break;
							}
						}

						ComponentCollisionAABB* collider2 = nullptr;
						for (Component* c : components2) {
							collider2 = dynamic_cast<ComponentCollisionAABB*>(c);
							if (collider2 != nullptr) {
								break;
							}
						}

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
		std::vector<glm::vec3> axes = GetAllCollisionAxis(transform, transform2);

		CollisionData collision;

		CollisionData bestCollision;
		bestCollision.collisionPenetration = -FLT_MAX;
		for (glm::vec3 axis : axes) {
			if (!CheckForCollisionOnAxis(axis, transform, dynamic_cast<ComponentCollisionBox*>(collider), transform2, dynamic_cast<ComponentCollisionAABB*>(collider2), collision)) {
				collision.isColliding = false;
				return collision;
			}

			if (collision.collisionPenetration >= bestCollision.collisionPenetration) {
				bestCollision = collision;
			}
			collision = CollisionData();
		}

		bestCollision.isColliding = true;

		return bestCollision;
	}
}