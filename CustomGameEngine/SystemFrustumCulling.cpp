#include "SystemFrustumCulling.h"
namespace Engine {
	SystemFrustumCulling::~SystemFrustumCulling()
	{

	}

	void SystemFrustumCulling::Run(const std::vector<Entity*>& entityList)
	{
		viewFrustum = &activeCamera->GetViewFrustum();

		System::Run(entityList);

		CullMeshes();
		CullReflectionProbes();
	}

	void SystemFrustumCulling::OnAction(Entity* entity)
	{

	}

	void SystemFrustumCulling::AfterAction()
	{

	}

	bool SystemFrustumCulling::AABBIsOnOrInFrontOfPlane(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin, const ViewPlane& plane)
	{
		// Convert min / max structuer to world space centre and centre to edge extents
		glm::vec3 worldMin = boxWorldOrigin + glm::vec3(aabb.minX, aabb.minY, aabb.minZ);
		glm::vec3 worldMax = boxWorldOrigin + glm::vec3(aabb.maxX, aabb.maxY, aabb.maxZ);

		glm::vec3 c = (worldMax + worldMin) * 0.5f; // world space centre
		glm::vec3 e = worldMax - c; // Positive extents x, y, z axis from centre

		// Compute projection interval radius of aabb onto L(t)
		float r = e.x * glm::abs(plane.normal.x) + e.y * glm::abs(plane.normal.y) + e.z * glm::abs(plane.normal.z);

		// Find distance of box centre from plane
		float distanceToPlane = glm::dot(plane.normal, c) - plane.distance;

		return -r <= distanceToPlane;
	}

	bool SystemFrustumCulling::TestAABBAndViewPlane(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin, const ViewPlane& plane)
	{
		// Convert min / max structuer to world space centre and centre to edge extents
		glm::vec3 worldMin = boxWorldOrigin + glm::vec3(aabb.minX, aabb.minY, aabb.minZ);
		glm::vec3 worldMax = boxWorldOrigin + glm::vec3(aabb.maxX, aabb.maxY, aabb.maxZ);

		glm::vec3 c = (worldMax + worldMin) * 0.5f; // world space centre
		glm::vec3 e = worldMax - c; // Positive extents x, y, z axis from centre

		// Compute projection interval radius of aabb onto L(t)
		float r = e.x * glm::abs(plane.normal.x) + e.y * glm::abs(plane.normal.y) + e.z * glm::abs(plane.normal.z);

		// Find distance of box centre from plane
		float distanceToPlane = glm::dot(plane.normal, c) - plane.distance;

		// If distance is within [-r, r] interval, collision is true
		return glm::abs(distanceToPlane) <= r;
	}

	bool SystemFrustumCulling::AABBIsInFrustum(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin)
	{
		geometryAABBTests++;
		return (AABBIsOnOrInFrontOfPlane(aabb, boxWorldOrigin, viewFrustum->left) && AABBIsOnOrInFrontOfPlane(aabb, boxWorldOrigin, viewFrustum->right) &&
				AABBIsOnOrInFrontOfPlane(aabb, boxWorldOrigin, viewFrustum->far) && AABBIsOnOrInFrontOfPlane(aabb, boxWorldOrigin, viewFrustum->near) &&
				AABBIsOnOrInFrontOfPlane(aabb, boxWorldOrigin, viewFrustum->top) && AABBIsOnOrInFrontOfPlane(aabb, boxWorldOrigin, viewFrustum->bottom));
	}

	bool SystemFrustumCulling::SphereIsOnOrInFrontOfPlane(const glm::vec3& spherePos, const float sphereRadius, const ViewPlane& plane)
	{
		float distanceToPlane = glm::dot(plane.normal, spherePos) - plane.distance;

		return distanceToPlane >= -sphereRadius;
	}


	void SystemFrustumCulling::CullMeshes()
	{
		visibleMeshes = 0;
		geometryAABBTests = 0;
		// Traverse BVH tree and check collisions with each node until a leaf node is found or no collision
		BVHTree* geometryBVH = collisionManager->GetBVHTree();
		TestBVHNodeRecursive(geometryBVH->GetRootNode());

		std::cout << "Visible meshes: " << visibleMeshes << " || Number of AABB tests: " << geometryAABBTests << std::endl;
	}

	void SystemFrustumCulling::TestBVHNodeRecursive(const BVHNode* node)
	{
		AABBPoints nodeAABB = node->GetBoundingBox();
		if (AABBIsInFrustum(nodeAABB, glm::vec3(0.0f))) {
			if (node->IsLeaf()) {
				// Check meshes
				std::vector<std::pair<glm::vec3, Mesh*>> nodeObjects = node->GetObjects();
				for (unsigned int i = 0; i < nodeObjects.size(); i++) {
					glm::vec3 origin = nodeObjects[i].first;
					Mesh* mesh = nodeObjects[i].second;
					AABBPoints geometryAABB = mesh->GetGeometryAABB();

					if (geometryAABB == nodeAABB || AABBIsInFrustum(geometryAABB, origin)) {
						visibleMeshes++;
					}
				}
			}
			else {
				// Check child nodes
				const BVHNode* leftChild = node->GetLeftChild();
				const BVHNode* rightChild = node->GetRightChild();
				if (leftChild) { TestBVHNodeRecursive(leftChild); }
				if (rightChild) { TestBVHNodeRecursive(rightChild); }
			}
		}
	}

	void SystemFrustumCulling::CullReflectionProbes()
	{
		RenderManager* renderManager = RenderManager::GetInstance();
		std::vector<ReflectionProbe*> reflectionProbes = renderManager->GetBakedData().GetReflectionProbes();
		culledProbeList.clear();

		for (ReflectionProbe* probe : reflectionProbes) {
			float soiRadius = probe->GetSOIRadius();
			glm::vec3 worldPos = probe->GetWorldPosition();

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