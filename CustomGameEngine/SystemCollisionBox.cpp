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

	CollisionData SystemCollisionBox::Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		std::vector<glm::vec3> axes = GetAllCollisionAxis(transform, transform2);

		CollisionData collision;
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

	void SystemCollisionBox::GetContactPoints(CollisionData& out_collisionInfo)
	{
		std::vector<glm::vec3> poly1, poly2;
		glm::vec3 normal1, normal2;
		glm::vec3 adjPlaneNormal1;
		glm::vec3 adjPlaneNormal2;
		float adjPlaneDistance1;
		float adjPlaneDistance2;

		// Get incident reference polygon 1
		glm::mat4 modelMatrix1 = dynamic_cast<ComponentTransform*>(out_collisionInfo.objectA->GetComponent(COMPONENT_TRANSFORM))->GetWorldModelMatrix();
		std::vector<glm::vec3> cube1 = dynamic_cast<ComponentCollisionBox*>(out_collisionInfo.objectA->GetComponent(COMPONENT_COLLISION_BOX))->WorldSpacePoints(modelMatrix1);
		// Get incident reference polygon 2

		// return 0 if either polygon contains no contact points

		// if poly 1 contacts == 1
		//		add contact point to collision

		// if poly 2 contacts == 1
		//		add contact point to collision

		// else
		//		clipping method
	}

	void SystemCollisionBox::GetIncidentReferencePolygon(const glm::vec3& axis, std::vector<glm::vec3>& out_face, glm::vec3& out_normal, glm::vec3& out_adjPlaneNormal1, glm::vec3& out_adjPlaneNormal2, float& out_adjPlaneDistance1, float& out_adjPlaneDistance2, const std::vector<glm::vec3>& worldPoints)
	{
		// Get furthest vertex along axis - furthest face
		float min;
		float max;
		int minVertexIndex;
		int maxVertexIndex;
		GetMinMaxOnAxis(worldPoints, axis, min, max, minVertexIndex, maxVertexIndex);

		// Get face which is furthest along axis (contains the furthest vertex)
		// Determined by normal being closest to parallel with axis

	}
}