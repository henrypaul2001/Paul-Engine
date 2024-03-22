#pragma once
#include "Entity.h"
#include <glm/ext/vector_float3.hpp>
namespace Engine {
	struct Collision {
		Entity* collidingObject;
		Entity* otherCollidingObject;

		glm::vec3 localCollisionPoint;
		glm::vec3 otherLocalCollisionPoint;

		glm::vec3 collisionNormal;

		float collisionPenetration;
	};

	class CollisionManager
	{
	public:
		CollisionManager();
		~CollisionManager();

		std::vector<Collision> GetUnresolvedCollisions() { return unresolvedCollisions; }
		void ClearUnresolvedCollisions() { unresolvedCollisions.clear(); }

		void AddToCollisionList(Collision newCollision) { unresolvedCollisions.push_back(newCollision); }
	private:
		std::vector<Collision> unresolvedCollisions;
	};
}