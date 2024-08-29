#include "BVHTree.h"
namespace Engine {
	BVHTree::BVHTree(unsigned int maxObjectsPerNode) : maxObjectsPerNode(maxObjectsPerNode)
	{
		rootNode = new BVHNode();
		nodeCount = 0;
	}

	BVHTree::~BVHTree()
	{
		delete rootNode;
	}

	void BVHTree::BuildTree(const std::vector<std::pair<glm::vec3, Mesh*>>& unsortedObjects)
	{
		AABBPoints worldAABB;
		nodeCount = 0;

		if (CreateNodeAABB(unsortedObjects, worldAABB)) {
			rootNode->SetBoundingBox(worldAABB);
			rootNode->SetParent(nullptr);

			// Build tree recursively
			BuildTreeRecursive(unsortedObjects, rootNode);
		}
	}

	bool BVHTree::BuildTreeRecursive(const std::vector<std::pair<glm::vec3, Mesh*>>& unsortedObjects, BVHNode* parentNode)
	{
		if (unsortedObjects.size() > 0u) {
			unsigned int excludedAxis = 3u;
			unsigned int attempts = 0u;

			AABBPoints parentBounds = parentNode->GetBoundingBox();

			bool success = false;
			bool attemptedX = false, attemptedY = false, attemptedZ = false;
			while (attempts < 2u && !success) {
				// Find biggest extent
				std::pair<unsigned int, float> axisIndexAndExtent = GetBiggestExtentAxis(parentBounds, excludedAxis); // 0 = X, 1 = Y, 2 = Z

				success = SplitNode(axisIndexAndExtent.first, axisIndexAndExtent.second, unsortedObjects, parentBounds, parentNode);

				excludedAxis = axisIndexAndExtent.first;

				if (excludedAxis == 0) { attemptedX = true; }
				else if (excludedAxis == 1) { attemptedY = true; }
				else if (excludedAxis == 2) { attemptedZ = true; }

				attempts++;
			}

			if (!success) {
				if (!attemptedX) {
					success = SplitNode(0, parentBounds.maxX - parentBounds.minX, unsortedObjects, parentBounds, parentNode);
				}
				else if (!attemptedY) {
					success = SplitNode(1, parentBounds.maxY - parentBounds.minY, unsortedObjects, parentBounds, parentNode);
				}
				else if (!attemptedZ) {
					success = SplitNode(2, parentBounds.maxZ - parentBounds.minZ, unsortedObjects, parentBounds, parentNode);
				}
			}

			return success;
		}
	}
}