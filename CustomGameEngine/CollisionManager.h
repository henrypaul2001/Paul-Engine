#pragma once
#include "Entity.h"
#include <glm/ext/vector_float3.hpp>
namespace Engine {
	struct CollisionData {
		Entity* collidingObject;
		Entity* otherCollidingObject;

		glm::vec3 localCollisionPoint;
		glm::vec3 otherLocalCollisionPoint;

		glm::vec3 collisionNormal;

		float collisionPenetration;

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