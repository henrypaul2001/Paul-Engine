#include "SystemCollisionBox.h"
namespace Engine {
	SystemCollisionBox::SystemCollisionBox(EntityManager* entityManager, CollisionManager* collisionManager) : SystemCollision(entityManager, collisionManager)
	{

	}

	SystemCollisionBox::~SystemCollisionBox()
	{

	}

	void SystemCollisionBox::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
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

			// Loop through all over entities to find another box collider entity
			for (Entity* e : entityManager->Entities()) {
				if ((e->Mask() & MASK) == MASK) {
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

						ComponentCollisionBox* collider2 = nullptr;
						for (Component* c : components2) {
							collider2 = dynamic_cast<ComponentCollisionBox*>(c);
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

	void SystemCollisionBox::AfterAction()
	{
		// Loop through all collision entities and clear EntitiesCheckedThisFrame
		for (Entity* e : entityManager->Entities()) {
			if ((e->Mask() & MASK) == MASK) {
				dynamic_cast<ComponentCollisionBox*>(e->GetComponent(COMPONENT_COLLISION_BOX))->ClearEntitiesCheckedThisFrame();
			}
		}
	}

	bool SystemCollisionBox::BroadPhaseIntersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		// Use a quick sphere-sphere collision test to see if objects are close enough to check with more complex SAT collision detection
		ComponentCollisionBox* boxA = dynamic_cast<ComponentCollisionBox*>(collider);
		ComponentCollisionBox* boxB = dynamic_cast<ComponentCollisionBox*>(collider2);

		float aRadius = (boxA->GetLocalPoints().GetBiggestExtent() / 2.0f) * transform->GetBiggestScaleFactor();
		float bRadius = (boxB->GetLocalPoints().GetBiggestExtent() / 2.0f) * transform2->GetBiggestScaleFactor();

		float distanceSqr = glm::distance2(transform->GetWorldPosition(), transform2->GetWorldPosition());

		float combinedRadius = aRadius + bRadius;
		float combinedRadiusSqr = combinedRadius * combinedRadius;

		return (distanceSqr < combinedRadiusSqr) ? true : false;
	}

	CollisionData SystemCollisionBox::Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		CollisionData collision;
		if (!BroadPhaseIntersect(transform, collider, transform2, collider2)) {
			collision.objectA = transform->GetOwner();
			collision.objectB = transform2->GetOwner();
			collision.isColliding = false;
			return collision;
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