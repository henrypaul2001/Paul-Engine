#include "SystemCollisionSphere.h"
namespace Engine {
	SystemCollisionSphere::SystemCollisionSphere(EntityManager* entityManager)
	{
		this->entityManager = entityManager;
	}

	SystemCollisionSphere::~SystemCollisionSphere()
	{

	}

	void SystemCollisionSphere::OnAction(Entity* entity)
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

			ComponentCollisionSphere* collider = nullptr;
			for (Component* c : components) {
				collider = dynamic_cast<ComponentCollisionSphere*>(c);
				if (collider != nullptr) {
					break;
				}
			}

			// Loop through all over entities to find another AABB collider entity
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

						ComponentCollisionSphere* collider2 = nullptr;
						for (Component* c : components2) {
							collider2 = dynamic_cast<ComponentCollisionSphere*>(c);
							if (collider2 != nullptr) {
								break;
							}
						}

						// Check for collision
						SphereCollision(transform, collider, transform2, collider2);
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

	bool intersect(float scaledRadius1, float scaledRadius2, glm::vec3 position1, glm::vec3 position2) {
		float distance = glm::distance(position1, position2);

		float combinedRadius = scaledRadius1 + scaledRadius2;

		return (distance <= combinedRadius);
	}

	float getBiggestScaleFactorFromTransform(ComponentTransform* transform) {
		float biggestScale = transform->Scale().x;
		if (transform->Scale().y > biggestScale) {
			biggestScale = transform->Scale().y;
		}
		else if (transform->Scale().z > biggestScale) {
			biggestScale = transform->Scale().z;
		}

		return biggestScale;
	}

	void SystemCollisionSphere::SphereCollision(ComponentTransform* transform, ComponentCollisionSphere* collider, ComponentTransform* transform2, ComponentCollisionSphere* collider2)
	{
		collider->AddToEntitiesCheckedThisFrame(collider2->GetOwner());
		collider2->AddToEntitiesCheckedThisFrame(collider->GetOwner());

		if (collider->useDefaultCollisionResponse && collider2->useDefaultCollisionResponse) {
			float scaledRadius1 = collider->CollisionRadius() * getBiggestScaleFactorFromTransform(transform);
			float scaledRadius2 = collider2->CollisionRadius() * getBiggestScaleFactorFromTransform(transform2);

			if (intersect(scaledRadius1, scaledRadius2, transform->GetWorldPosition(), transform2->GetWorldPosition())) {
				SystemCollision::DefaultCollisionResponse(transform->GetOwner(), transform2->GetOwner());

				collider->AddToCollisions(collider2->GetOwner());
				collider2->AddToCollisions(collider->GetOwner());
			}
			else {
				collider->RemoveFromCollisions(collider2->GetOwner());
				collider2->RemoveFromCollisions(collider->GetOwner());
			}
		}
	}
}