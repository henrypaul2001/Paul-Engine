#pragma once
#include "System.h"
#include "Camera.h"
#include "RenderManager.h"
#include "CollisionManager.h"
#include <map>
namespace Engine {
	enum FrustumIntersection {
		OUTSIDE_FRUSTUM,
		INSIDE_FRUSTUM,
		PARTIAL_FRUSTUM,
	};

	class SystemFrustumCulling : public System
	{
	public:
		SystemFrustumCulling(Camera* activeCamera, CollisionManager* collisionManager) { 
			this->activeCamera = activeCamera;
			this->viewFrustum = &activeCamera->GetViewFrustum();
			this->collisionManager = collisionManager;
			visibleMeshes = 0;
			totalMeshes = 0;
			geometryAABBTests = 0;
		}
		~SystemFrustumCulling();

		SystemTypes Name() override { return SYSTEM_FRUSTUM_CULLING; }
		void Run(const std::vector<Entity*>& entityList) override;
		void OnAction(Entity* entity) override;
		void AfterAction() override;

		const unsigned int GetVisibleMeshes() const { return visibleMeshes; }
		const unsigned int GetTotalMeshes() const { return totalMeshes; }
		const unsigned int GetTotalAABBTests() const { return geometryAABBTests; }

		void SetActiveCamera(Camera* newCamera) { this->activeCamera = newCamera; }

		static std::map<float, Mesh*> culledMeshList;
	private:
		FrustumIntersection AABBIsOnOrInFrontOfPlane(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin, const ViewPlane& plane);
		bool TestAABBAndViewPlane(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin, const ViewPlane& plane);
		FrustumIntersection AABBIsInFrustum(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin);
		bool SphereIsOnOrInFrontOfPlane(const glm::vec3& spherePos, const float sphereRadius, const ViewPlane& plane);

		void CullMeshes();
		void AddMeshToCulledList(Mesh* mesh);
		void TestBVHNodeRecursive(const BVHNode* node, const FrustumIntersection& parentResult);
		void TestBVHNodeRecursive(const BVHNode* node);
		void CullReflectionProbes();

		const ComponentTypes GEOMETRY_MASK = (COMPONENT_GEOMETRY);

		Camera* activeCamera;
		const ViewFrustum* viewFrustum;
		
		CollisionManager* collisionManager;

		std::map<float, ReflectionProbe*> culledProbeList;
		const std::vector<BVHObject>* globalBVHObjectList;

		unsigned int visibleMeshes;
		unsigned int totalMeshes;
		unsigned int geometryAABBTests;
	};
}