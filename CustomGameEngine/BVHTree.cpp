#include "BVHTree.h"
namespace Engine {
	BVHTree::BVHTree()
	{
		rootNode = new BVHNode();
	}

	BVHTree::~BVHTree()
	{
		delete rootNode;
	}

	void BVHTree::BuildTree(const std::vector<std::pair<glm::vec3, Mesh*>>& unsortedObjects)
	{
		if (unsortedObjects.size() > 0) {
			// Calculate root bounding box
			glm::vec3 position = unsortedObjects[0].first;
			AABBPoints geoBounds = unsortedObjects[0].second->GetGeometryAABB();
			float minX = position.x + geoBounds.minX;
			float minY = position.y + geoBounds.minY;
			float minZ = position.z + geoBounds.minZ;
			float maxX = position.x + geoBounds.maxX;
			float maxY = position.y + geoBounds.maxY;
			float maxZ = position.z + geoBounds.maxZ;

			for (int i = 1; i < unsortedObjects.size(); i++) {
				geoBounds = unsortedObjects[i].second->GetGeometryAABB();
				position = unsortedObjects[i].first;

				if (position.x + geoBounds.minX < minX) { minX = position.x + geoBounds.minX; }
				if (position.x + geoBounds.maxX > maxX) { maxX = position.x + geoBounds.maxX; }

				if (position.y + geoBounds.minY < minY) { minY = position.y + geoBounds.minY; }
				if (position.y + geoBounds.maxY > maxY) { maxY = position.y + geoBounds.maxY; }

				if (position.z + geoBounds.minZ < minZ) { minZ = position.z + geoBounds.minZ; }
				if (position.z + geoBounds.maxZ > maxZ) { maxZ = position.z + geoBounds.maxZ; }
			}

			AABBPoints worldAABB = AABBPoints(minX, minY, minZ, maxX, maxY, maxZ);
			rootNode->SetBoundingBox(worldAABB);

			// Build tree recursively
			BuildTreeRecursive(unsortedObjects, rootNode);
		}
	}

	BVHNode* BVHTree::BuildTreeRecursive(const std::vector<std::pair<glm::vec3, Mesh*>>& unsortedObjects, BVHNode* parentNode)
	{
		// Find biggest extent
		AABBPoints parentBounds = parentNode->GetBoundingBox();
		std::pair<unsigned int, float> axisIndexAndExtent = GetBiggestExtentAxis(parentBounds); // 0 = X, 1 = Y, 2 = Z

		// Sort meshes from smallest position to biggest position on biggest extent axis
		std::vector<std::pair<glm::vec3, Mesh*>> sortedObjects = SortMeshesOnAxis(unsortedObjects, axisIndexAndExtent.first);

		// Split meshes into left and right
		// --------------------------------
		glm::vec3 parentMin = glm::vec3(parentBounds.minX, parentBounds.minY, parentBounds.minZ);
		float halfExtent = parentMin[axisIndexAndExtent.first] + axisIndexAndExtent.second * 0.5f;

		// Find split index
		unsigned int leftIndex = 0;
		unsigned int rightIndex = sortedObjects.size() - 1;
		unsigned int splitIndex = GetSplitIndex(sortedObjects, halfExtent, axisIndexAndExtent.first);



		return nullptr;
	}
}