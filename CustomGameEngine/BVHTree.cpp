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
		globalObjects.clear();
		AABBPoints worldAABB;
		nodeCount = 0;

		// Create global objects
		const int numObjects = unsortedObjects.size();
		std::vector<unsigned int> indices;
		indices.reserve(numObjects);
		globalObjects.reserve(numObjects);
		for (unsigned int i = 0; i < numObjects; i++) {
			std::pair<glm::vec3, Mesh*> pair = unsortedObjects[i];
			globalObjects.push_back(BVHObject(pair.second, pair.first, i));
			indices.push_back(i);
		}

		if (CreateNodeAABB(indices, worldAABB)) {
			rootNode->SetBoundingBox(worldAABB);
			rootNode->SetParent(nullptr);
			rootNode->SetObjectIndices(indices);
			rootNode->SetIsLeaf(false);

			// Build tree recursively
			bool success = BuildTreeRecursive(indices, rootNode);
			if (!success) {
				rootNode->SetIsLeaf(true);
			}
		}
	}

	bool BVHTree::BuildTreeRecursive(const std::vector<unsigned int>& indicesUnsorted, BVHNode* parentNode) {
		if (indicesUnsorted.size() > 0) {
			unsigned int excludedAxis = 3u;
			unsigned int attempts = 0u;

			AABBPoints parentBounds = parentNode->GetBoundingBox();

			bool success = false;
			bool attemptedX = false, attemptedY = false, attemptedZ = false;

			while (attempts < 2u && !success) {
				// Find biggest extent
				std::pair<unsigned int, float> axisIndexAndExtent = GetBiggestExtentAxis(parentBounds, excludedAxis); // 0 = X, 1 = Y, 2 = Z
			
				success = SplitNode(axisIndexAndExtent.first, axisIndexAndExtent.second, indicesUnsorted, parentBounds, parentNode);

				excludedAxis = axisIndexAndExtent.first;

				if (excludedAxis == 0) { attemptedX = true; }
				else if (excludedAxis == 1) { attemptedY = true; }
				else if (excludedAxis == 2) { attemptedZ = true; }

				attempts++;
			}

			if (!success) {
				if (!attemptedX) {
					success = SplitNode(0, parentBounds.maxX - parentBounds.minX, indicesUnsorted, parentBounds, parentNode);
				}
				else if (!attemptedY) {
					success = SplitNode(1, parentBounds.maxY - parentBounds.minY, indicesUnsorted, parentBounds, parentNode);
				}
				else if (!attemptedZ) {
					success = SplitNode(2, parentBounds.maxZ - parentBounds.minZ, indicesUnsorted, parentBounds, parentNode);
				}
			}

			return success;
		}
	}
}