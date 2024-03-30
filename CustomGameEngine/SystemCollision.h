#pragma once
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentCollision.h"
#include "ComponentCollisionAABB.h"
#include "ComponentCollisionBox.h"
#include "ComponentCollisionSphere.h"
#include "EntityManager.h"
#include "CollisionManager.h"
namespace Engine {
	class SystemCollision : public System
	{
	protected:
		EntityManager* entityManager;
		CollisionManager* collisionManager;
		void Collision(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2);
		virtual CollisionData Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2) = 0;
		void DefaultCollisionResponse(Entity* entity1, Entity* entity2);

		bool CheckForCollisionOnAxis(glm::vec3 axis, ComponentTransform* transform, ComponentCollisionBox* collider, ComponentTransform* transform2, ComponentCollisionBox* collider2, CollisionData& collision);
		bool CheckForCollisionOnAxis(glm::vec3 axis, ComponentTransform* transform, ComponentCollisionBox* collider, ComponentTransform* transform2, ComponentCollisionAABB* collider2, CollisionData& collision);
		std::vector<glm::vec3> GetCubeNormals(ComponentTransform* transform);
		std::vector<glm::vec3> GetEdgeVectors(ComponentTransform* transform);
		std::vector<glm::vec3> GetAllCollisionAxis(ComponentTransform* transform, ComponentTransform* transform2);
	public:
		SystemCollision(EntityManager* entityManager, CollisionManager* collisionManager);
		~SystemCollision();

		void GetMinMaxOnAxis(const std::vector<glm::vec3>& worldSpacePoints, const glm::vec3& worldSpaceAxis, float& out_min, float& out_max);

		virtual SystemTypes Name() override = 0;
		virtual void OnAction(Entity* entity) override = 0;
		virtual void AfterAction() override = 0;
	};
}