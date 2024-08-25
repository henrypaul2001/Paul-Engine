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
			float minX = unsortedObjects[0].second->GetGeometryAABB().minX;
			float minY = unsortedObjects[0].second->GetGeometryAABB().minY;
			float minZ = unsortedObjects[0].second->GetGeometryAABB().minZ;
			float maxX = unsortedObjects[0].second->GetGeometryAABB().maxX;
			float maxY = unsortedObjects[0].second->GetGeometryAABB().maxY;
			float maxZ = unsortedObjects[0].second->GetGeometryAABB().maxZ;

			for (int i = 1; i < unsortedObjects.size(); i++) {
				AABBPoints geoBounds = unsortedObjects[i].second->GetGeometryAABB();

				if (geoBounds.minX < minX) { minX = geoBounds.minX; }
				if (geoBounds.maxX > maxX) { maxX = geoBounds.maxX; }

				if (geoBounds.minY < minY) { minY = geoBounds.minY; }
				if (geoBounds.maxY > maxY) { maxY = geoBounds.maxY; }

				if (geoBounds.minZ < minZ) { minZ = geoBounds.minZ; }
				if (geoBounds.maxZ > maxZ) { maxZ = geoBounds.maxZ; }
			}

			AABBPoints worldAABB = AABBPoints(minX, minY, minZ, maxX, maxY, maxZ);
			rootNode->SetBoundingBox(worldAABB);

			// Build tree recursively
		}
	}
}