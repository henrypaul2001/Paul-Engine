#include "SystemCollision.h"
#include "ComponentTransform.h"
namespace Engine {
	SystemCollision::SystemCollision(EntityManager* entityManager, CollisionManager* collisionManager)
	{
		this->entityManager = entityManager;
		this->collisionManager = collisionManager;
	}

	SystemCollision::~SystemCollision()
	{

	}

	void SystemCollision::Collision(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2)
	{
		collider->AddToEntitiesCheckedThisFrame(collider2->GetOwner());
		collider2->AddToEntitiesCheckedThisFrame(collider->GetOwner());

		if (collider->useDefaultCollisionResponse && collider2->useDefaultCollisionResponse) {
			CollisionData collision = Intersect(transform, collider, transform2, collider2);
			if (collision.isColliding) {
				//DefaultCollisionResponse(transform->GetOwner(), transform2->GetOwner());
				collisionManager->AddToCollisionList(collision);

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

	bool SystemCollision::CheckForCollisionOnAxis(glm::vec3 axis, ComponentTransform* transform, ComponentCollisionBox* collider, ComponentTransform* transform2, ComponentCollisionBox* collider2, CollisionData& collision)
	{
		float cube1Min;
		float cube2Min;

		float cube1Max;
		float cube2Max;

		std::vector<glm::vec3> cube1 = collider->WorldSpacePoints(transform->GetWorldModelMatrix());
		std::vector<glm::vec3> cube2 = collider2->WorldSpacePoints(transform2->GetWorldModelMatrix());

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

		glm::vec3 closestPoint = glm::vec3();
		//closestPoint.x = std::max(localMinMax.minX, std::min(transformedSherePosition.x, localMinMax.maxX));
		//closestPoint.y = std::max(localMinMax.minY, std::min(transformedSherePosition.y, localMinMax.maxY));
		//closestPoint.z = std::max(localMinMax.minZ, std::min(transformedSherePosition.z, localMinMax.maxZ));

		if (cube1Min <= cube2Min && cube1Max >= cube2Min) {
			collision.collisionNormal = glm::normalize(axis);
			collision.collisionPenetration = cube2Min - cube1Max;
			collision.otherLocalCollisionPoint = cube1Max + collision.collisionNormal * collision.collisionPenetration;
			//collision.localCollisionPoint = glm::vec3();
			//collision.otherLocalCollisionPoint = glm::vec3();
			return true;
		}

		if (cube2Min <= cube1Min && cube2Max >= cube1Min) {
			collision.collisionNormal = -glm::normalize(axis);
			collision.collisionPenetration = cube1Min - cube2Max;
			collision.otherLocalCollisionPoint = cube1Min + collision.collisionNormal * collision.collisionPenetration;
			//collision.localCollisionPoint = glm::vec3();
			//collision.otherLocalCollisionPoint = glm::vec3();
			return true;
		}

		return false;
	}

	bool SystemCollision::CheckForCollisionOnAxis(glm::vec3 axis, ComponentTransform* transform, ComponentCollisionBox* collider, ComponentTransform* transform2, ComponentCollisionAABB* collider2, CollisionData& collision)
	{
		float cube1Min;
		float cube2Min;

		float cube1Max;
		float cube2Max;

		std::vector<glm::vec3> cube1 = collider->WorldSpacePoints(transform->GetWorldModelMatrix());
		std::vector<glm::vec3> cube2 = collider2->WorldSpacePoints(transform2->GetWorldModelMatrix());

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

		if (cube1Min <= cube2Min && cube1Max >= cube2Min) {
			collision.collisionNormal = glm::normalize(axis);
			collision.collisionPenetration = cube2Min - cube1Max;
			collision.otherLocalCollisionPoint = cube2Min + collision.collisionNormal * collision.collisionPenetration;
			//collision.otherLocalCollisionPoint = cube1Max + collision.collisionNormal * collision.collisionPenetration;
			return true;
		}

		if (cube2Min <= cube1Min && cube2Max >= cube1Min) {
			collision.collisionNormal = -glm::normalize(axis);
			collision.collisionPenetration = cube1Min - cube2Max;
			collision.otherLocalCollisionPoint = cube1Min + collision.collisionNormal * collision.collisionPenetration;
			//collision.localCollisionPoint = cube2Max + collision.collisionNormal * collision.collisionPenetration;
			return true;
		}

		return false;
	}

	std::vector<glm::vec3> SystemCollision::GetCubeNormals(ComponentTransform* transform)
	{
		glm::mat3 rotationMatrix = glm::mat3(transform->GetWorldModelMatrix());

		std::vector<glm::vec3> normals;
		normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f)); // right face
		normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f)); // top face
		normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f)); // front face

		// Rotate each normal based on objects current rotation
		for (glm::vec3& normal : normals) {
			normal = glm::normalize(rotationMatrix * normal);
		}

		return normals;
	}

	std::vector<glm::vec3> SystemCollision::GetEdgeVectors(ComponentTransform* transform)
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

	std::vector<glm::vec3> SystemCollision::GetAllCollisionAxis(ComponentTransform* transform, ComponentTransform* transform2)
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