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

	void SystemCollision::GetMinMaxOnAxis(const std::vector<glm::vec3>& worldSpacePoints, const glm::vec3& worldSpaceAxis, float& out_min, float& out_max)
	{
		if (worldSpacePoints.size() < 1) {
			return;
		}

		out_min = glm::dot(worldSpacePoints[0], worldSpaceAxis);
		out_max = out_min;

		float projectedPosition;
		for (unsigned int i = 1; i < worldSpacePoints.size(); i++) {
			projectedPosition = glm::dot(worldSpacePoints[i], worldSpaceAxis);
			if (projectedPosition > out_max) {
				out_max = projectedPosition;
			}
			else if (projectedPosition < out_min) {
				out_min = projectedPosition;
			}
		}
	}

	void SystemCollision::GetMinMaxOnAxis(const std::vector<glm::vec3>& worldSpacePoints, const glm::vec3& worldSpaceAxis, float& out_min, float& out_max, int& out_minIndex, int& out_maxIndex)
	{
		if (worldSpacePoints.size() < 1) {
			return;
		}

		out_min = glm::dot(worldSpacePoints[0], worldSpaceAxis);
		out_minIndex = 0;
		out_max = out_min;
		out_maxIndex = out_minIndex;

		float projectedPosition;
		for (unsigned int i = 1; i < worldSpacePoints.size(); i++) {
			projectedPosition = glm::dot(worldSpacePoints[i], worldSpaceAxis);
			if (projectedPosition > out_max) {
				out_max = projectedPosition;
				out_maxIndex = i;
			}
			else if (projectedPosition < out_min) {
				out_min = projectedPosition;
				out_minIndex = i;
			}
		}
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
		GetMinMaxOnAxis(cube1, axis, cube1Min, cube1Max);
		GetMinMaxOnAxis(cube2, axis, cube2Min, cube2Max);

		if (cube1Min <= cube2Min && cube1Max >= cube2Min) {
			glm::vec3 collisionNormal = glm::normalize(axis);
			float collisionPenetration = cube2Min - cube1Max;
			glm::vec3 otherLocalCollisionPoint = cube1Max + collisionNormal * collisionPenetration;

			collision.AddContactPoint(glm::vec3(), otherLocalCollisionPoint, collisionNormal, collisionPenetration);
			return true;
		}

		if (cube2Min <= cube1Min && cube2Max >= cube1Min) {
			glm::vec3 collisionNormal = -glm::normalize(axis);
			float collisionPenetration = cube1Min - cube2Max;
			glm::vec3 otherLocalCollisionPoint = cube1Min + collisionNormal * collisionPenetration;

			collision.AddContactPoint(glm::vec3(), otherLocalCollisionPoint, collisionNormal, collisionPenetration);
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
		GetMinMaxOnAxis(cube1, axis, cube1Min, cube1Max);
		GetMinMaxOnAxis(cube2, axis, cube2Min, cube2Max);

		if (cube1Min <= cube2Min && cube1Max >= cube2Min) {
			glm::vec3 collisionNormal = glm::normalize(axis);
			float collisionPenetration = cube2Min - cube1Max;
			glm::vec3 otherLocalCollisionPoint = cube2Min + collisionNormal * collisionPenetration;

			collision.AddContactPoint(glm::vec3(), otherLocalCollisionPoint, collisionNormal, collisionPenetration);
			return true;
		}

		if (cube2Min <= cube1Min && cube2Max >= cube1Min) {
			glm::vec3 collisionNormal = -glm::normalize(axis);
			float collisionPenetration = cube1Min - cube2Max;
			glm::vec3 otherLocalCollisionPoint = cube1Min + collisionNormal * collisionPenetration;
			
			collision.AddContactPoint(glm::vec3(), otherLocalCollisionPoint, collisionNormal, collisionPenetration);
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