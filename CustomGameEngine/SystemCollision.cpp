#include "SystemCollision.h"
#include "ComponentTransform.h"
namespace Engine {
	SystemCollision::SystemCollision(EntityManager* entityManager)
	{
		this->entityManager = entityManager;
	}

	SystemCollision::~SystemCollision()
	{

	}

	void SystemCollision::Collision(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		collider->AddToEntitiesCheckedThisFrame(collider2->GetOwner());
		collider2->AddToEntitiesCheckedThisFrame(collider->GetOwner());

		if (collider->useDefaultCollisionResponse && collider2->useDefaultCollisionResponse) {
			if (Intersect(transform, collider, transform2, collider2)) {
				DefaultCollisionResponse(transform->GetOwner(), transform2->GetOwner());

				collider->AddToCollisions(collider2->GetOwner());
				collider2->AddToCollisions(collider->GetOwner());
			}
			else {
				collider->RemoveFromCollisions(collider2->GetOwner());
				collider2->RemoveFromCollisions(collider->GetOwner());
			}
		}
	}

	void SystemCollision::DefaultCollisionResponse(Entity* entity1, Entity* entity2)
	{
		ComponentTransform* transform1 = dynamic_cast<ComponentTransform*>(entity1->GetComponent(COMPONENT_TRANSFORM));
		transform1->SetPosition(transform1->LastPosition());

		ComponentTransform* transform2 = dynamic_cast<ComponentTransform*>(entity2->GetComponent(COMPONENT_TRANSFORM));
		transform2->SetPosition(transform2->LastPosition());
	}
}