#include "SystemCollisionBox.h"
namespace Engine {
	SystemCollisionBox::SystemCollisionBox(EntityManager* entityManager) : SystemCollision(entityManager)
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

	bool SystemCollisionBox::Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		std::vector<glm::vec3> axes = GetAllCollisionAxis(transform, transform2);

		for (glm::vec3 axis : axes) {
			if (!CheckForCollisionOnAxis(axis, transform, collider, transform2, collider2)) {
				return false;
			}
		}

		return true;
	}

	bool SystemCollisionBox::CheckForCollisionOnAxis(glm::vec3 axis, ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		float cube1Min;
		float cube2Min;

		float cube1Max;
		float cube2Max;

		std::vector<glm::vec3> cube1 = dynamic_cast<ComponentCollisionBox*>(collider)->WorldSpacePoints(transform);
		std::vector<glm::vec3> cube2 = dynamic_cast<ComponentCollisionBox*>(collider2)->WorldSpacePoints(transform2);

		// Project points onto axis and check for overlap
		cube1Min = glm::dot(cube1[0], axis);
		cube1Max = cube1Min;

		cube2Min = glm::dot(cube2[0], axis);
		cube2Max = cube2Min;

		float projectedPositionOnAxis;
		for (unsigned int i = 1; i < cube1.size(); i++) {
			// Cube 1
			projectedPositionOnAxis = glm::dot(cube1[i], axis);
			if (projectedPositionOnAxis > cube1Max) {
				cube1Max = projectedPositionOnAxis;
			}
			else if (projectedPositionOnAxis < cube1Min) {
				cube1Min = projectedPositionOnAxis;
			}

			// Cube 2
			projectedPositionOnAxis = glm::dot(cube2[i], axis);
			if (projectedPositionOnAxis > cube2Max) {
				cube2Max = projectedPositionOnAxis;
			}
			else if (projectedPositionOnAxis < cube2Min) {
				cube2Min = projectedPositionOnAxis;
			}
		}

		return (cube1Min <= cube2Max && cube1Max >= cube2Min);
	}

	std::vector<glm::vec3> SystemCollisionBox::GetCubeNormals(ComponentTransform* transform)
	{
		glm::mat3 rotationMatrix = glm::mat3(transform->GetWorldModelMatrix());

		std::vector<glm::vec3> normals;
		normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f)); // right face
		normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f)); // top face
		normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f)); // front face

		// Rotate each normal based on objects current rotation
		for (glm::vec3& normal : normals) {
			normal = rotationMatrix * normal;
		}

		return normals;
	}

	std::vector<glm::vec3> SystemCollisionBox::GetEdgeVectors(ComponentTransform* transform)
	{
		glm::mat3 rotationMatrix = glm::mat3(transform->GetWorldModelMatrix());

		std::vector<glm::vec3> edges;
		edges.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
		edges.push_back(glm::vec3(-1.0f, 1.0f, 1.0f));
		edges.push_back(glm::vec3(1.0f, -1.0f, 1.0f));
		edges.push_back(glm::vec3(-1.0f, -1.0f, 1.0f));
		edges.push_back(glm::vec3(1.0f, 1.0f, -1.0f));
		edges.push_back(glm::vec3(-1.0f, 1.0f, -1.0f));
		edges.push_back(glm::vec3(1.0f, -1.0f, -1.0f));
		edges.push_back(glm::vec3(-1.0f, -1.0f, -1.0f));
		edges.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
		edges.push_back(glm::vec3(1.0f, -1.0f, 1.0f));
		edges.push_back(glm::vec3(-1.0f, 1.0f, 1.0f));
		edges.push_back(glm::vec3(-1.0f, -1.0f, 1.0f));

		// Rotate each edge by objects current rotation
		for (glm::vec3& edge : edges) {
			edge = rotationMatrix * edge;
		}

		return edges;
	}

	std::vector<glm::vec3> SystemCollisionBox::GetAllCollisionAxis(ComponentTransform* transform, ComponentTransform* transform2)
	{
		std::vector<glm::vec3> axes;

		// Get normals of both cubes
		std::vector<glm::vec3> cube1Normals = GetCubeNormals(transform);
		std::vector<glm::vec3> cube2Normals = GetCubeNormals(transform2);

		// Get edge vectors of both cubes
		//std::vector<glm::vec3> cube1Edges = GetEdgeVectors(transform);
		//std::vector<glm::vec3> cube2Edges = GetEdgeVectors(transform2);

		// Combine into one list of potential collision axes
		axes = cube1Normals;
		axes.insert(axes.end(), cube2Normals.begin(), cube2Normals.end());

		/*
		// Add cross product of all edges to axes
		for (glm::vec3 edge1 : cube1Edges) {
			for (glm::vec3 edge2 : cube2Edges) {
				axes.push_back(glm::cross(edge1, edge2));
			}
		}
		*/
		
		// Add cross product of all normals to axes
		for (glm::vec3 normal1 : cube1Normals) {
			for (glm::vec3 normal2 : cube2Normals) {
				axes.push_back(glm::cross(normal1, normal2));
			}
		}
		
		int size = axes.size();
		return axes;
	}
}