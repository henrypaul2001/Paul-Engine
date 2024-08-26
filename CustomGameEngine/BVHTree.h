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
		BVHTree();
		~BVHTree();

		void BuildTree(const std::vector<std::pair<glm::vec3, Mesh*>>& unsortedObjects);

		BVHNode* GetRootNode() { return rootNode; }
	private:
		BVHNode* BuildTreeRecursive(const std::vector<std::pair<glm::vec3, Mesh*>>& unsortedObjects, BVHNode* parentNode);

		std::pair<unsigned int, float> GetBiggestExtentAxis(const AABBPoints& aabb) {
			float extents[3]{
				aabb.maxX - aabb.minX,
				aabb.maxY - aabb.minY,
				aabb.maxZ - aabb.minZ
			};

			unsigned int biggestExtent = 0;
			if (extents[1] > extents[0] && extents[1] > extents[2]) { biggestExtent = 1; }
			if (extents[2] > extents[0] && extents[2] > extents[1]) { biggestExtent = 2; }

			return std::make_pair(biggestExtent, extents[biggestExtent]);
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
			for (int i = 0; i < sortedObjects.size(); i++) {
				glm::vec3 position = sortedObjects[i].first;
				if (position[axisIndex] >= halfExtent) {
					return i;
				}
			}
			return sortedObjects.size() - 1;
		}

		BVHNode* rootNode;
	};
}