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

	void BVHTree::BuildTree(const std::vector<std::pair<const glm::vec3, Mesh*>>& unsortedObjects)
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
		}
	}
}