#pragma once
#include "Entity.h"
#include <glm/ext/vector_float3.hpp>
namespace Engine {
	struct ContactPoint {
		ContactPoint(glm::vec3 contactA, glm::vec3 contactB, glm::vec3 collisionNormal, float collisionPenetration) : contactPointA(contactA), contactPointB(contactB), normal(collisionNormal), penetration(collisionPenetration) {}

		glm::vec3 contactPointA;
		glm::vec3 contactPointB;

		glm::vec3 normal;

		float penetration;
	};

	struct CollisionData {
		Entity* objectA;
		Entity* objectB;

		std::vector<ContactPoint> contactPoints;

		void AddContactPoint(glm::vec3 contactA, glm::vec3 contactB, glm::vec3 normal, float penetration) {
			contactPoints.push_back(ContactPoint(contactA, contactB, normal, penetration));
		};

		bool isColliding;
	};

	class CollisionManager
	{
	public:
		CollisionManager();
		~CollisionManager();

		std::vector<CollisionData> GetUnresolvedCollisions() { return unresolvedCollisions; }
		void ClearUnresolvedCollisions() { unresolvedCollisions.clear(); }

		void AddToCollisionList(CollisionData newCollision) { unresolvedCollisions.push_back(newCollision); }
	private:
		std::vector<CollisionData> unresolvedCollisions;
	};
}