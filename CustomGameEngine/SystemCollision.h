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
	struct Edge {
		Edge(glm::vec3 start = glm::vec3(0.0f), glm::vec3 end = glm::vec3(0.0f)) : start(start), end(end) {}

		glm::vec3 start;
		glm::vec3 end;
	};

	class SystemCollision : public System
	{
	protected:
		EntityManager* entityManager;
		CollisionManager* collisionManager;
		void Collision(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2);
		virtual CollisionData Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2) = 0;

		void GetContactPoints(CollisionData& out_collisionInfo);
		void GetIncidentReferencePolygon(const glm::vec3& axis, std::vector<glm::vec3>& out_face, glm::vec3& out_normal, std::vector<ClippingPlane>& out_adjPlanes, Entity* object);
		bool CheckForCollisionOnAxis(glm::vec3 axis, ComponentTransform* transform, ComponentCollisionBox* collider, ComponentTransform* transform2, ComponentCollisionBox* collider2, CollisionData& collision);
		bool CheckForCollisionOnAxis(glm::vec3 axis, ComponentTransform* transform, ComponentCollisionBox* collider, ComponentTransform* transform2, ComponentCollisionAABB* collider2, CollisionData& collision);
		std::vector<glm::vec3> GetCubeNormals(ComponentTransform* transform);
		std::vector<glm::vec3> GetEdgeVectors(ComponentTransform* transform);
		std::vector<glm::vec3> GetAllCollisionAxis(ComponentTransform* transform, ComponentTransform* transform2);
		void SutherlandHodgmanClipping(const std::vector<glm::vec3>& input_polygon, int num_clip_planes, const ClippingPlane* clip_planes, std::vector<glm::vec3>* out_polygon, bool removeNotClipToPlane);
		bool PlaneEdgeIntersection(const ClippingPlane& plane, const glm::vec3& start, const glm::vec3& end, glm::vec3& out_point);
		glm::vec3 GetClosestPointPolygon(const glm::vec3& pos, const std::vector<glm::vec3>& polygon);
		glm::vec3 GetClosestPoint(const glm::vec3& pos, std::vector<Edge>& edges);
		glm::vec3 GetClosestPoint(const glm::vec3& pos, Edge& edge);
	public:
		SystemCollision(EntityManager* entityManager, CollisionManager* collisionManager);
		~SystemCollision();

		void GetMinMaxOnAxis(const std::vector<glm::vec3>& worldSpacePoints, const glm::vec3& worldSpaceAxis, float& out_min, float& out_max);
		void GetMinMaxOnAxis(const std::vector<glm::vec3>& worldSpacePoints, const glm::vec3& worldSpaceAxis, float& out_min, float& out_max, int& out_minIndex, int& out_maxIndex);

		virtual SystemTypes Name() override = 0;
		virtual void OnAction(Entity* entity) override = 0;
		virtual void AfterAction() override = 0;
	};
}