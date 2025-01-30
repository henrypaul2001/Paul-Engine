#pragma once
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentCollision.h"
#include "ComponentCollisionAABB.h"
#include "ComponentCollisionBox.h"
#include "ComponentCollisionSphere.h"
#include "CollisionManager.h"
namespace Engine {
	struct Edge {
		Edge(const glm::vec3& start = glm::vec3(0.0f), const glm::vec3& end = glm::vec3(0.0f)) : start(start), end(end) {}

		glm::vec3 start;
		glm::vec3 end;
	};

	class SystemCollision : public System
	{
	public:
		SystemCollision(EntityManagerNew* ecs, CollisionManager* collisionManager) : System(ecs), collisionManager(collisionManager) {}
		~SystemCollision() {}

		void GetMinMaxOnAxis(const std::vector<glm::vec3>& worldSpacePoints, const glm::vec3& worldSpaceAxis, float& out_min, float& out_max) const;
		void GetMinMaxOnAxis(const std::vector<glm::vec3>& worldSpacePoints, const glm::vec3& worldSpaceAxis, float& out_min, float& out_max, int& out_minIndex, int& out_maxIndex) const;

		virtual constexpr const char* SystemName() override = 0;

	protected:
		CollisionManager* collisionManager;

		void CollisionPreCheck(const unsigned int entityIDA, ComponentCollision* colliderA, const unsigned int entityIDB, ComponentCollision* colliderB) const {
			colliderA->AddToEntitiesCheckedThisFrame(entityIDB, active_ecs->Find(entityIDB)->Name());
			colliderB->AddToEntitiesCheckedThisFrame(entityIDA, active_ecs->Find(entityIDA)->Name());
		}

		void CollisionPostCheck(const CollisionData& collision, const unsigned int entityIDA, ComponentCollision* colliderA, const unsigned int entityIDB, ComponentCollision* colliderB) {
			if (collision.isColliding) {
				collisionManager->AddToCollisionList(collision);

				colliderA->AddToCollisions(entityIDB, active_ecs->Find(entityIDB)->Name());
				colliderB->AddToCollisions(entityIDA, active_ecs->Find(entityIDA)->Name());
			}
			else {
				colliderA->RemoveFromCollisions(entityIDB);
				colliderB->RemoveFromCollisions(entityIDA);
			}
		}

		void GetContactPoints(CollisionData& out_collisionInfo) const;
		void GetIncidentReferencePolygon(const glm::vec3& axis, std::vector<glm::vec3>& out_face, glm::vec3& out_normal, std::vector<ClippingPlane>& out_adjPlanes, const unsigned int entityID) const;
		bool CheckForCollisionOnAxis(const glm::vec3& axis, const ComponentTransform& transform, const ComponentCollisionBox& collider, const ComponentTransform& transform2, const ComponentCollisionBox& collider2, CollisionData& collision) const;
		bool CheckForCollisionOnAxis(const glm::vec3& axis, const ComponentTransform& transform, const ComponentCollisionBox& collider, const ComponentTransform& transform2, const ComponentCollisionAABB& collider2, CollisionData& collision) const;
		std::vector<glm::vec3> GetCubeNormals(const ComponentTransform& transform) const;
		std::vector<glm::vec3> GetEdgeVectors(const ComponentTransform& transform) const;
		std::vector<glm::vec3> GetAllCollisionAxis(const ComponentTransform& transform, const ComponentTransform& transform2) const;
		void SutherlandHodgmanClipping(const std::vector<glm::vec3>& input_polygon, int num_clip_planes, const ClippingPlane* clip_planes, std::vector<glm::vec3>* out_polygon, const bool removeNotClipToPlane) const;
		bool PlaneEdgeIntersection(const ClippingPlane& plane, const glm::vec3& start, const glm::vec3& end, glm::vec3& out_point) const;
		glm::vec3 GetClosestPointPolygon(const glm::vec3& pos, const std::vector<glm::vec3>& polygon) const;
		glm::vec3 GetClosestPoint(const glm::vec3& pos, std::vector<Edge>& edges) const;
		glm::vec3 GetClosestPoint(const glm::vec3& pos, Edge& edge) const;

		bool BroadPhaseSphereSphere(const ComponentTransform& transform, const ComponentCollisionBox& collider, const ComponentTransform& transform2, const ComponentCollisionBox& collider2) const;
		bool BroadPhaseSphereSphere(const ComponentTransform& transform, const ComponentCollisionBox& collider, const ComponentTransform& transform2, const ComponentCollisionAABB& collider2) const;
		bool BroadPhaseSphereSphere(const ComponentTransform& transform, const ComponentCollisionAABB& collider, const ComponentTransform& transform2, const ComponentCollisionAABB& collider2) const;
	};
}