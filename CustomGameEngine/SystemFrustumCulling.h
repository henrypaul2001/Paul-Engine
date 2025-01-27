#pragma once
#include "ReflectionProbe.h"
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

	// This is a special case system which doesn't operate per entity/component set but on all meshes in the scene. Therefore, this system doesn't get registered to the system manager in the same way as other systems
	class SystemFrustumCulling
	{
	public:
		SystemFrustumCulling() : activeCamera(nullptr), collisionManager(nullptr), visibleMeshes(0), totalMeshes(0), geometryAABBTests(0) {}
		~SystemFrustumCulling() {}

		void Run(Camera* activeCamera, CollisionManager* collisionManager);

		const unsigned int GetVisibleMeshes() const { return visibleMeshes; }
		const unsigned int GetTotalMeshes() const { return totalMeshes; }
		const unsigned int GetTotalAABBTests() const { return geometryAABBTests; }

		//void SetActiveCamera(Camera* newCamera) { this->activeCamera = newCamera; }

		static std::map<float, std::pair<Mesh*, unsigned int>> culledMeshList;
	private:
		FrustumIntersection AABBIsOnOrInFrontOfPlane(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin, const ViewPlane& plane);
		bool TestAABBAndViewPlane(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin, const ViewPlane& plane);
		FrustumIntersection AABBIsInFrustum(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin);
		bool SphereIsOnOrInFrontOfPlane(const glm::vec3& spherePos, const float sphereRadius, const ViewPlane& plane);

		void CullMeshes();
		void AddMeshToCulledList(const BVHObject& bvhObject);
		void TestBVHNodeRecursive(const BVHNode* node);
		void CullReflectionProbes();

		Camera* activeCamera;
		ViewFrustum viewFrustum;
		
		CollisionManager* collisionManager;

		std::map<float, ReflectionProbe*> culledProbeList;
		const std::vector<BVHObject>* globalBVHObjectList;

		unsigned int visibleMeshes;
		unsigned int totalMeshes;
		unsigned int geometryAABBTests;
	};
}