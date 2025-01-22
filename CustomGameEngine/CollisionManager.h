#pragma once
//#include "Entity.h"
#include <glm/ext/vector_float3.hpp>
#include "BVHTree.h"
namespace Engine {
	struct ContactPoint {
		ContactPoint(const glm::vec3& contactA, const glm::vec3& contactB, const glm::vec3& collisionNormal, const float collisionPenetration) : contactPointA(contactA), contactPointB(contactB), normal(collisionNormal), penetration(collisionPenetration), b_term(0.0f), sumImpulseContact(0.0f), sumImpulseFriction(glm::vec3(0.0f)) {}

		glm::vec3 contactPointA;
		glm::vec3 contactPointB;

		glm::vec3 normal;

		float penetration;

		float b_term;
		glm::vec3 sumImpulseFriction;
		float sumImpulseContact;
	};

	struct CollisionData {
		unsigned int entityIDA;
		unsigned int entityIDB;

		std::vector<ContactPoint> contactPoints;

		void AddContactPoint(const glm::vec3& contactA, const glm::vec3& contactB, const glm::vec3& normal, const float penetration) {
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

		void ConstructBVHTree();

		BVHTree* GetBVHTree() { return bvhTree; }
	private:
		std::vector<CollisionData> unresolvedCollisions;

		BVHTree* bvhTree;
	};
}