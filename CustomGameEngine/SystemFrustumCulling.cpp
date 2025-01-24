#include "SystemFrustumCulling.h"
#include <glm/gtx/norm.hpp>
namespace Engine {
	std::map<float, std::pair<Mesh*, unsigned int>> SystemFrustumCulling::culledMeshList = std::map<float, std::pair<Mesh*, unsigned int>>();

	void SystemFrustumCulling::Run(Camera* activeCamera, CollisionManager* collisionManager)
	{
		SCOPE_TIMER("SystemFrustumCulling::Run");
		this->activeCamera = activeCamera;
		this->collisionManager = collisionManager;
		viewFrustum = &activeCamera->GetViewFrustum();
		totalMeshes = 0;
		visibleMeshes = 0;
		geometryAABBTests = 0;

		CullMeshes();
		CullReflectionProbes();
	}

	FrustumIntersection SystemFrustumCulling::AABBIsOnOrInFrontOfPlane(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin, const ViewPlane& plane)
	{
		// Convert min / max structuer to world space centre and centre to edge extents
		const glm::vec3 worldMin = boxWorldOrigin + glm::vec3(aabb.minX, aabb.minY, aabb.minZ);
		const glm::vec3 worldMax = boxWorldOrigin + glm::vec3(aabb.maxX, aabb.maxY, aabb.maxZ);

		const glm::vec3 c = (worldMax + worldMin) * 0.5f; // world space centre
		const glm::vec3 e = worldMax - c; // Positive extents x, y, z axis from centre

		// Compute projection interval radius of aabb onto L(t)
		const float r = e.x * glm::abs(plane.normal.x) + e.y * glm::abs(plane.normal.y) + e.z * glm::abs(plane.normal.z);

		// Find distance of box centre from plane
		const float distanceToPlane = glm::dot(plane.normal, c) - plane.distance;

		if (-r > distanceToPlane) { return OUTSIDE_FRUSTUM; }
		else if (r >= distanceToPlane) { return PARTIAL_FRUSTUM; }
		else { return INSIDE_FRUSTUM; }
	}

	bool SystemFrustumCulling::TestAABBAndViewPlane(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin, const ViewPlane& plane)
	{
		// Convert min / max structuer to world space centre and centre to edge extents
		const glm::vec3 worldMin = boxWorldOrigin + glm::vec3(aabb.minX, aabb.minY, aabb.minZ);
		const glm::vec3 worldMax = boxWorldOrigin + glm::vec3(aabb.maxX, aabb.maxY, aabb.maxZ);

		const glm::vec3 c = (worldMax + worldMin) * 0.5f; // world space centre
		const glm::vec3 e = worldMax - c; // Positive extents x, y, z axis from centre

		// Compute projection interval radius of aabb onto L(t)
		const float r = e.x * glm::abs(plane.normal.x) + e.y * glm::abs(plane.normal.y) + e.z * glm::abs(plane.normal.z);

		// Find distance of box centre from plane
		const float distanceToPlane = glm::dot(plane.normal, c) - plane.distance;

		// If distance is within [-r, r] interval, collision is true
		return glm::abs(distanceToPlane) <= r;
	}

	FrustumIntersection SystemFrustumCulling::AABBIsInFrustum(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin)
	{
		SCOPE_TIMER("SystemFrustumCulling::AABBIsInFrustum");
		geometryAABBTests++;
		bool fullyInside = true;
		for (const ViewPlane& plane : { viewFrustum->left, viewFrustum->right, viewFrustum->far, viewFrustum->near, viewFrustum->top, viewFrustum->bottom }) {
			const FrustumIntersection planeResult = AABBIsOnOrInFrontOfPlane(aabb, boxWorldOrigin, plane);

			if (planeResult == OUTSIDE_FRUSTUM) { return planeResult; }
			else if (planeResult == PARTIAL_FRUSTUM) { fullyInside = false; }
		}

		return fullyInside ? INSIDE_FRUSTUM : PARTIAL_FRUSTUM;
	}

	bool SystemFrustumCulling::SphereIsOnOrInFrontOfPlane(const glm::vec3& spherePos, const float sphereRadius, const ViewPlane& plane)
	{
		const float distanceToPlane = glm::dot(plane.normal, spherePos) - plane.distance;

		return distanceToPlane >= -sphereRadius;
	}


	void SystemFrustumCulling::CullMeshes()
	{
		SCOPE_TIMER("SystemFrustumCulling::CullMeshes");
		culledMeshList.clear();
		// Traverse BVH tree and check collisions with each node until a leaf node is found or no collision
		BVHTree* geometryBVH = collisionManager->GetBVHTree();
		BVHNode* rootNode = geometryBVH->GetRootNode();
		globalBVHObjectList = &geometryBVH->GetGlobalObjects();

		TestBVHNodeRecursive(rootNode);

		visibleMeshes = culledMeshList.size();
		totalMeshes = globalBVHObjectList->size();
	}

	void SystemFrustumCulling::AddMeshToCulledList(const BVHObject& bvhObject)
	{
		SCOPE_TIMER("SystemFrustumCulling::AddMeshToCulledList");
		const glm::vec3& meshOrigin = bvhObject.worldPosition;

		// Get distance to mesh
		float distanceToCameraSquared = glm::distance2(activeCamera->GetPosition(), meshOrigin);
		int iterations = 0;
		while (culledMeshList.find(distanceToCameraSquared) != culledMeshList.end()) {
			// Distance already exists, increment slightly
			distanceToCameraSquared += 0.01f;
			iterations++;
			if (iterations > 100) {
				std::cout << "ahhhhhh" << std::endl;
			}
		}
		culledMeshList[distanceToCameraSquared] = std::make_pair(bvhObject.mesh, bvhObject.entityID);
	}

	void SystemFrustumCulling::TestBVHNodeRecursive(const BVHNode* node)
	{
		SCOPE_TIMER("SystemFrustumCulling::TestBVHNodeRecursive");
		const AABBPoints& nodeAABB = node->GetBoundingBox();

		const FrustumIntersection nodeIsInFrustum = AABBIsInFrustum(nodeAABB, glm::vec3(0.0f));
		
		if (nodeIsInFrustum != OUTSIDE_FRUSTUM) {
			// Full intersection, all following children will also be inside frustum
			if (nodeIsInFrustum == INSIDE_FRUSTUM) {
				// Add all node meshes to culled list
				const std::vector<unsigned int>& indices = node->GetGlobalObjectIndices();
				for (unsigned int i : indices) {
					AddMeshToCulledList(globalBVHObjectList->at(i));
				}
			}
			else {
				// Partially inside frustum, check children
				if (node->IsLeaf()) {
					// Test meshes
					const std::vector<unsigned int>& indices = node->GetGlobalObjectIndices();
					for (unsigned int i = 0; i < indices.size(); i++) {
						const glm::vec3& origin = globalBVHObjectList->at(indices[i]).worldPosition;
						const BVHObject& bvhObject = globalBVHObjectList->at(indices[i]);
						const Mesh* mesh = bvhObject.mesh;
						const AABBPoints& geometryAABB = mesh->GetGeometryAABB();

						if (geometryAABB == nodeAABB || AABBIsInFrustum(geometryAABB, origin)) {
							AddMeshToCulledList(bvhObject);
						}
					}
				}
				else {
					// Test children
					const BVHNode* leftChild = node->GetLeftChild();
					const BVHNode* rightChild = node->GetRightChild();
					if (leftChild) { TestBVHNodeRecursive(leftChild); }
					if (rightChild) { TestBVHNodeRecursive(rightChild); }
				}
			}
		}
	}

	void SystemFrustumCulling::CullReflectionProbes()
	{
		SCOPE_TIMER("SystemFrustumCulling::CullReflectionProbes");
		RenderManager* renderManager = RenderManager::GetInstance();
		const std::vector<ReflectionProbe*>& reflectionProbes = renderManager->GetBakedData().GetReflectionProbes();
		culledProbeList.clear();

		for (ReflectionProbe* probe : reflectionProbes) {
			SCOPE_TIMER("SystemFrustumCulling::CullReflectionProbes::TestProbe");
			const float soiRadius = probe->GetSOIRadius();
			const glm::vec3& worldPos = probe->GetWorldPosition();

			// Check for collision on each view plane, starting with most likely to fail first
			if (SphereIsOnOrInFrontOfPlane(worldPos, soiRadius, viewFrustum->left) && SphereIsOnOrInFrontOfPlane(worldPos, soiRadius, viewFrustum->right) &&
				SphereIsOnOrInFrontOfPlane(worldPos, soiRadius, viewFrustum->far) && SphereIsOnOrInFrontOfPlane(worldPos, soiRadius, viewFrustum->near) &&
				SphereIsOnOrInFrontOfPlane(worldPos, soiRadius, viewFrustum->top) && SphereIsOnOrInFrontOfPlane(worldPos, soiRadius, viewFrustum->bottom))
			{
				// Probe is inside view frustum
				float distanceToCameraSquared = glm::distance2(activeCamera->GetPosition(), worldPos);
				if (culledProbeList.find(distanceToCameraSquared) != culledProbeList.end()) {
					// Distance already exists, increment slightly
					distanceToCameraSquared += 0.00001f;
				}
				culledProbeList[distanceToCameraSquared] = probe;
			}
		}

		renderManager->GetBakedData().SetCulledProbeList(culledProbeList);
	}
}