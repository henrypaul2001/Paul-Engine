#pragma once
#include "BVHNode.h"
#include "Mesh.h"
#include <algorithm>
namespace Engine {

	static bool ComparePositionX(std::pair<glm::vec3, Mesh*> entryOne, std::pair<const glm::vec3, Mesh*> entryTwo) {
		return (entryOne.first.x < entryTwo.first.x);
	}

	static bool ComparePositionY(std::pair<glm::vec3, Mesh*> entryOne, std::pair<const glm::vec3, Mesh*> entryTwo) {
		return (entryOne.first.y < entryTwo.first.y);
	}

	static bool ComparePositionZ(std::pair<glm::vec3, Mesh*> entryOne, std::pair<const glm::vec3, Mesh*> entryTwo) {
		return (entryOne.first.z < entryTwo.first.z);
	}

	class BVHTree
	{
	public:
		BVHTree(unsigned int maxObjectsPerNode = 3u);
		~BVHTree();

		void BuildTree(const std::vector<std::pair<glm::vec3, Mesh*>>& unsortedObjects);

		BVHNode* GetRootNode() { return rootNode; }
	private:
		// Returns false if parent node could not be split on any axis
		bool BuildTreeRecursive(const std::vector<std::pair<glm::vec3, Mesh*>>& unsortedObjects, BVHNode* parentNode);

		// 3 == no exclusion
		std::pair<unsigned int, float> GetBiggestExtentAxis(const AABBPoints& aabb, const unsigned int excludedAxis = 3u) {
			unsigned int biggestExtent;
			float extents[3]{
				aabb.maxX - aabb.minX,
				aabb.maxY - aabb.minY,
				aabb.maxZ - aabb.minZ
			};

			switch (excludedAxis) {
			case 0:
				// Exclude X
				biggestExtent = 1;
				if (extents[2] > extents[1]) { biggestExtent = 2; }

				return std::make_pair(biggestExtent, extents[biggestExtent]);
				break;
			case 1:
				// Exclude Y
				biggestExtent = 0;
				if (extents[2] > extents[0]) { biggestExtent = 2; }

				return std::make_pair(biggestExtent, extents[biggestExtent]);
				break;
			case 2:
				// Exclude Z
				biggestExtent = 0;
				if (extents[1] > extents[0]) { biggestExtent = 1; }

				return std::make_pair(biggestExtent, extents[biggestExtent]);
				break;
			case 3:
				// No excluded axis
				biggestExtent = 0;
				if (extents[1] > extents[0] && extents[1] > extents[2]) { biggestExtent = 1; }
				if (extents[2] > extents[0] && extents[2] > extents[1]) { biggestExtent = 2; }

				return std::make_pair(biggestExtent, extents[biggestExtent]);
				break;
			}
		}

		std::vector<std::pair<glm::vec3, Mesh*>> SortMeshesOnAxis(const std::vector<std::pair<glm::vec3, Mesh*>>& unsortedObjects, unsigned int axisIndex) {
			std::vector<std::pair<glm::vec3, Mesh*>> sorted = unsortedObjects;

			switch (axisIndex) {
			case 0:
				std::sort(sorted.begin(), sorted.end(), ComparePositionX);
				break;
			case 1:
				std::sort(sorted.begin(), sorted.end(), ComparePositionY);
				break;
			case 2:
				std::sort(sorted.begin(), sorted.end(), ComparePositionZ);
				break;
			default:
				break;
			}

			return sorted;
		}

		unsigned int GetSplitIndex(const std::vector<std::pair<glm::vec3, Mesh*>>& sortedObjects, float halfExtent, unsigned int axisIndex) {
			if (sortedObjects.size() == 0) {
				return 0;
			}

			for (int i = 0; i < sortedObjects.size(); i++) {
				glm::vec3 position = sortedObjects[i].first;
				if (position[axisIndex] >= halfExtent) {
					return i;
				}
			}
			return sortedObjects.size() - 1;
		}

		bool SplitNode(unsigned int axisIndex, float axisExtent, const std::vector<std::pair<glm::vec3, Mesh*>>& unsortedObjects, const AABBPoints& parentBounds, BVHNode* parentNode) {
			AABBPoints leftAABB, rightAABB;

			// Sort meshes from smallest position to biggest position on biggest extent axis
			std::vector<std::pair<glm::vec3, Mesh*>> sortedObjects = SortMeshesOnAxis(unsortedObjects, axisIndex);

			// Split meshes into left and right
			// --------------------------------
			glm::vec3 parentMin = glm::vec3(parentBounds.minX, parentBounds.minY, parentBounds.minZ);
			float halfExtent = parentMin[axisIndex] + axisExtent * 0.5f;

			// Find split index
			unsigned int splitIndex = GetSplitIndex(sortedObjects, halfExtent, axisIndex);

			if (splitIndex == 0) {
				// Split axis didn't work
				return false;
			}

			// Create AABB for left and right nodes
			std::vector<std::pair<glm::vec3, Mesh*>> leftSplit(sortedObjects.begin(), sortedObjects.begin() + splitIndex);
			std::vector<std::pair<glm::vec3, Mesh*>> rightSplit(sortedObjects.begin() + splitIndex, sortedObjects.end());

			int leftSuccess = CreateNodeAABB(leftSplit, leftAABB);
			int rightSuccess = CreateNodeAABB(rightSplit, rightAABB);

			if (leftSuccess) {
				parentNode->SetLeftChild(BuildNode(parentNode, leftSplit, leftAABB));
			}

			if (rightSuccess) {
				parentNode->SetRightChild(BuildNode(parentNode, rightSplit, rightAABB));
			}

			return true;
		}

		bool CreateNodeAABB(const std::vector<std::pair<glm::vec3, Mesh*>>& objects, AABBPoints& out_aabb) {
			if (objects.size() > 0) {
				// Calculate root bounding box
				glm::vec3 position = objects[0].first;
				AABBPoints geoBounds = objects[0].second->GetGeometryAABB();
				float minX = position.x + geoBounds.minX;
				float minY = position.y + geoBounds.minY;
				float minZ = position.z + geoBounds.minZ;
				float maxX = position.x + geoBounds.maxX;
				float maxY = position.y + geoBounds.maxY;
				float maxZ = position.z + geoBounds.maxZ;

				for (int i = 1; i < objects.size(); i++) {
					geoBounds = objects[i].second->GetGeometryAABB();
					position = objects[i].first;

					if (position.x + geoBounds.minX < minX) { minX = position.x + geoBounds.minX; }
					if (position.x + geoBounds.maxX > maxX) { maxX = position.x + geoBounds.maxX; }

					if (position.y + geoBounds.minY < minY) { minY = position.y + geoBounds.minY; }
					if (position.y + geoBounds.maxY > maxY) { maxY = position.y + geoBounds.maxY; }

					if (position.z + geoBounds.minZ < minZ) { minZ = position.z + geoBounds.minZ; }
					if (position.z + geoBounds.maxZ > maxZ) { maxZ = position.z + geoBounds.maxZ; }
				}

				out_aabb = AABBPoints(minX, minY, minZ, maxX, maxY, maxZ);
				return true;
			}
			else {
				return false;
			}
		}

		BVHNode* BuildNode(BVHNode* parent, const std::vector<std::pair<glm::vec3, Mesh*>>& objects, const AABBPoints aabb) {
			BVHNode* node = nullptr;
			if (objects.size() > 0u) {
				node = new BVHNode();
				node->SetBoundingBox(aabb);
				node->SetParent(parent);

				// Check number of objects in this node
				if (objects.size() <= maxObjectsPerNode) {
					node->SetIsLeaf(true);
					node->SetObjects(objects);
				}
				else {
					// Split node
					bool success = BuildTreeRecursive(objects, node);
					if (!success) {
						node->SetIsLeaf(true);
						node->SetObjects(objects);
					}
				}
			}
			return node;
		}

		BVHNode* rootNode;
		unsigned int maxObjectsPerNode;
	};
}