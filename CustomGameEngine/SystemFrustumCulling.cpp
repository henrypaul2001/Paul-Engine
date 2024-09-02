#include "SystemFrustumCulling.h"
namespace Engine {
	SystemFrustumCulling::~SystemFrustumCulling()
	{

	}

	void SystemFrustumCulling::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemFrustumCulling::Run");
		viewFrustum = &activeCamera->GetViewFrustum();
		totalMeshes = 0;
		visibleMeshes = 0;
		geometryAABBTests = 0;
		System::Run(entityList);

		CullMeshes();
		CullReflectionProbes();
	}

	void SystemFrustumCulling::OnAction(Entity* entity)
	{
		SCOPE_TIMER("SystemFrustumCulling::OnAction");
		// Reset isVisible flag on all meshes
		if ((entity->Mask() & GEOMETRY_MASK) == GEOMETRY_MASK) {
			std::vector<Component*> components = entity->Components();

			ComponentGeometry* geometry = nullptr;
			for (Component* c : components) {
				geometry = dynamic_cast<ComponentGeometry*>(c);
				if (geometry != nullptr) {
					break;
				}
			}

			Model* model = geometry->GetModel();
			for (Mesh* m : model->meshes) {
				totalMeshes++;
				m->SetIsVisible(false);
			}
		}
	}

	void SystemFrustumCulling::AfterAction()
	{

	}

	FrustumIntersection SystemFrustumCulling::AABBIsOnOrInFrontOfPlane(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin, const ViewPlane& plane)
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

		if (-r > distanceToPlane) { return OUTSIDE_FRUSTUM; }
		else if (r >= distanceToPlane) { return PARTIAL_FRUSTUM; }
		else { return INSIDE_FRUSTUM; }
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

	FrustumIntersection SystemFrustumCulling::AABBIsInFrustum(const AABBPoints& aabb, const glm::vec3& boxWorldOrigin)
	{
		SCOPE_TIMER("SystemFrustumCulling::AABBIsInFrustum");
		geometryAABBTests++;
		bool fullyInside = true;
		for (const ViewPlane& plane : { viewFrustum->left, viewFrustum->right, viewFrustum->far, viewFrustum->near, viewFrustum->top, viewFrustum->bottom }) {
			FrustumIntersection planeResult = AABBIsOnOrInFrontOfPlane(aabb, boxWorldOrigin, plane);

			if (planeResult == OUTSIDE_FRUSTUM) { return planeResult; }
			else if (planeResult == PARTIAL_FRUSTUM) { fullyInside = false; }
		}

		return fullyInside ? INSIDE_FRUSTUM : PARTIAL_FRUSTUM;
	}

	bool SystemFrustumCulling::SphereIsOnOrInFrontOfPlane(const glm::vec3& spherePos, const float sphereRadius, const ViewPlane& plane)
	{
		float distanceToPlane = glm::dot(plane.normal, spherePos) - plane.distance;

		return distanceToPlane >= -sphereRadius;
	}


	void SystemFrustumCulling::CullMeshes()
	{
		SCOPE_TIMER("SystemFrustumCulling::CullMeshes");
		// Traverse BVH tree and check collisions with each node until a leaf node is found or no collision
		BVHTree* geometryBVH = collisionManager->GetBVHTree();
		BVHNode* rootNode = geometryBVH->GetRootNode();

		FrustumIntersection nodeResult = AABBIsInFrustum(rootNode->GetBoundingBox(), glm::vec3(0.0f));

		if (nodeResult != OUTSIDE_FRUSTUM) {
			// Check child nodes
			const BVHNode* leftChild = rootNode->GetLeftChild();
			const BVHNode* rightChild = rootNode->GetRightChild();
			if (leftChild) { TestBVHNodeRecursive(leftChild, nodeResult); }
			if (rightChild) { TestBVHNodeRecursive(rightChild, nodeResult); }
		}
	}

	void SystemFrustumCulling::TestBVHNodeRecursive(const BVHNode* node, const FrustumIntersection& parentResult)
	{
		SCOPE_TIMER("SystemFrustumCulling::TestBVHNodeRecursive");
		AABBPoints nodeAABB = node->GetBoundingBox();

		// Only test node AABB if parent result was partially inside frustum
		FrustumIntersection nodeIsInFrustum = parentResult;
		if (nodeIsInFrustum == PARTIAL_FRUSTUM) {
			nodeIsInFrustum = AABBIsInFrustum(nodeAABB, glm::vec3(0.0f));
		}

		if (nodeIsInFrustum != OUTSIDE_FRUSTUM) {
			if (node->IsLeaf()) {
				std::vector<std::pair<glm::vec3, Mesh*>> nodeObjects = node->GetObjects();
				if (nodeIsInFrustum == PARTIAL_FRUSTUM) {
					// Check all meshes
					for (unsigned int i = 0; i < nodeObjects.size(); i++) {
						glm::vec3 origin = nodeObjects[i].first;
						Mesh* mesh = nodeObjects[i].second;
						AABBPoints geometryAABB = mesh->GetGeometryAABB();

						if (geometryAABB == nodeAABB || AABBIsInFrustum(geometryAABB, origin)) {
							visibleMeshes++;
							mesh->SetIsVisible(true);
						}
					}
				}
				else {
					// Skip checks, all will be true
					for (const std::pair<glm::vec3, Mesh*>& pair : nodeObjects) {
						visibleMeshes++;
						pair.second->SetIsVisible(true);
					}
				}
			}
			else {
				// Check child nodes
				const BVHNode* leftChild = node->GetLeftChild();
				const BVHNode* rightChild = node->GetRightChild();
				if (leftChild) { TestBVHNodeRecursive(leftChild, nodeIsInFrustum); }
				if (rightChild) { TestBVHNodeRecursive(rightChild, nodeIsInFrustum); }
			}
		}
	}

	void SystemFrustumCulling::CullReflectionProbes()
	{
		SCOPE_TIMER("SystemFrustumCulling::CullReflectionProbes");
		RenderManager* renderManager = RenderManager::GetInstance();
		std::vector<ReflectionProbe*> reflectionProbes = renderManager->GetBakedData().GetReflectionProbes();
		culledProbeList.clear();

		for (ReflectionProbe* probe : reflectionProbes) {
			SCOPE_TIMER("SystemFrustumCulling::CullReflectionProbes::TestProbe");
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