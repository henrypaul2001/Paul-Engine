#include "SystemCollisionSphereBox.h"
namespace Engine {
	SystemCollisionSphereBox::SystemCollisionSphereBox(EntityManager* entityManager, CollisionManager* collisionManager) : SystemCollision(entityManager, collisionManager)
	{

	}

	SystemCollisionSphereBox::~SystemCollisionSphereBox()
	{

	}

	void SystemCollisionSphereBox::Run(const std::vector<Entity*>& entityList)
	{
	}

	void SystemCollisionSphereBox::OnAction(Entity* entity)
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
				if ((e->Mask() & SPHERE_MASK) == SPHERE_MASK) {
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
						Collision(transform, collider, transform2, collider2);
					}
				}
			}
		}
	}

	void SystemCollisionSphereBox::AfterAction()
	{
		// Loop through all collision entities and clear EntitiesCheckedThisFrame
		for (Entity* e : entityManager->Entities()) {
			if ((e->Mask() & BOX_MASK) == BOX_MASK) {
				dynamic_cast<ComponentCollisionBox*>(e->GetComponent(COMPONENT_COLLISION_BOX))->ClearEntitiesCheckedThisFrame();
			}
			else if ((e->Mask() & SPHERE_MASK) == SPHERE_MASK) {
				dynamic_cast<ComponentCollisionSphere*>(e->GetComponent(COMPONENT_COLLISION_SPHERE))->ClearEntitiesCheckedThisFrame();
			}
		}
	}

	CollisionData SystemCollisionSphereBox::Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		// Transform sphere into oriented box's local space
		glm::vec3 worldSpaceSpherePosition = transform2->GetWorldPosition();
		glm::mat4 boxTransform = transform->GetWorldModelMatrix();

		glm::vec3 transformedSherePosition = glm::vec3(glm::inverse(boxTransform) * glm::vec4(worldSpaceSpherePosition, 1.0f));

		// Find closest point on box to sphere position
		BoxExtents localMinMax = dynamic_cast<ComponentCollisionBox*>(collider)->GetLocalPoints();
		glm::vec3 closestPoint = glm::vec3();
		closestPoint.x = std::max(localMinMax.minX, std::min(transformedSherePosition.x, localMinMax.maxX));
		closestPoint.y = std::max(localMinMax.minY, std::min(transformedSherePosition.y, localMinMax.maxY));
		closestPoint.z = std::max(localMinMax.minZ, std::min(transformedSherePosition.z, localMinMax.maxZ));

		// Transform closest point into world space
		glm::vec3 closestPointWorldSpace = glm::vec3(boxTransform * glm::vec4(closestPoint, 1.0f));

		float distance = glm::distance(closestPointWorldSpace, worldSpaceSpherePosition);

		float scaledRadius = dynamic_cast<ComponentCollisionSphere*>(collider2)->CollisionRadius() * transform2->GetBiggestScaleFactor();

		CollisionData collision;
		if (distance < scaledRadius) {
			collision.isColliding = true;

			float collisionPenetration = scaledRadius - distance;
			glm::vec3 collisionNormal = -glm::normalize(transform2->GetWorldPosition() - closestPointWorldSpace);
			glm::vec3 localCollisionPoint = closestPoint;
			glm::vec3 otherLocalCollisionPoint = -collisionNormal * scaledRadius;
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