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
		SystemFrustumCulling(Camera* activeCamera, CollisionManager* collisionManager) { this->activeCamera = activeCamera; this->viewFrustum = &activeCamera->GetViewFrustum(); this->collisionManager = collisionManager; }
		~SystemFrustumCulling();

		SystemTypes Name() override { return SYSTEM_FRUSTUM_CULLING; }
		void Run(const std::vector<Entity*>& entityList) override;
		void OnAction(Entity* entity) override;
		void AfterAction() override;

		void SetActiveCamera(Camera* newCamera) { this->activeCamera = newCamera; }
	private:
		FrustumIntersection AABBIsOnOrInFrontOfPlane(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin, const ViewPlane& plane);
		bool TestAABBAndViewPlane(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin, const ViewPlane& plane);
		FrustumIntersection AABBIsInFrustum(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin);
		bool SphereIsOnOrInFrontOfPlane(const glm::vec3& spherePos, const float sphereRadius, const ViewPlane& plane);

		void CullMeshes();
		void TestBVHNodeRecursive(const BVHNode* node, const FrustumIntersection& parentResult);
		void CullReflectionProbes();

		const ComponentTypes GEOMETRY_MASK = (COMPONENT_GEOMETRY);

		Camera* activeCamera;
		const ViewFrustum* viewFrustum;
		
		CollisionManager* collisionManager;

		std::map<float, ReflectionProbe*> culledProbeList;

		unsigned int visibleMeshes;
		unsigned int totalMeshes;
		unsigned int geometryAABBTests;
	};
}