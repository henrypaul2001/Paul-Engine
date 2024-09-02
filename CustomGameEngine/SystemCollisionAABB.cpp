#include "SystemCollisionAABB.h"
#include <iostream>
namespace Engine {
	SystemCollisionAABB::SystemCollisionAABB(EntityManager* entityManager, CollisionManager* collisionManager) : SystemCollision(entityManager, collisionManager)
	{
	}

	SystemCollisionAABB::~SystemCollisionAABB()
	{
	}

	void SystemCollisionAABB::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemCollisionAABB::Run");
		System::Run(entityList);
	}

	void SystemCollisionAABB::OnAction(Entity* entity)
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

			ComponentCollisionAABB* collider = nullptr;
			for (Component* c : components) {
				collider = dynamic_cast<ComponentCollisionAABB*>(c);
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

	void SystemCollisionAABB::AfterAction()
	{
		// Loop through all collision entities and clear EntitiesCheckedThisFrame
		for (Entity* e : entityManager->Entities()) {
			if ((e->Mask() & MASK) == MASK) {
				dynamic_cast<ComponentCollisionAABB*>(e->GetComponent(COMPONENT_COLLISION_AABB))->ClearEntitiesCheckedThisFrame();
			}
		}
	}

	CollisionData SystemCollisionAABB::Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		AABBPoints bounds1 = dynamic_cast<ComponentCollisionAABB*>(collider)->GetWorldSpaceBounds(transform->GetWorldModelMatrix());
		AABBPoints bounds2 = dynamic_cast<ComponentCollisionAABB*>(collider2)->GetWorldSpaceBounds(transform2->GetWorldModelMatrix());

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
			
			float distances[6] = {
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

			collision.objectA = transform->GetOwner();
			collision.objectB = transform2->GetOwner();

			GetContactPoints(collision);
		}
		else {
			collision.isColliding = false;
		}

		return collision;
	}
}