#pragma once
#include "BVHNode.h"
#include "Mesh.h"
#include <algorithm>
namespace Engine {

	static bool StaticComparePositionX(std::pair<glm::vec3, Mesh*> entryOne, std::pair<const glm::vec3, Mesh*> entryTwo) {
		return (entryOne.first.x < entryTwo.first.x);
	}
	static bool StaticComparePositionY(std::pair<glm::vec3, Mesh*> entryOne, std::pair<const glm::vec3, Mesh*> entryTwo) {
		return (entryOne.first.y < entryTwo.first.y);
	}
	static bool StaticComparePositionZ(std::pair<glm::vec3, Mesh*> entryOne, std::pair<const glm::vec3, Mesh*> entryTwo) {
		return (entryOne.first.z < entryTwo.first.z);
	}

	struct BVHObject {
		Mesh* mesh;
		glm::vec3 worldPosition;
		unsigned int globalIndex;
		unsigned int entityID;

		BVHObject(Mesh* mesh, const glm::vec3& worldPosition, const unsigned int index, const unsigned int entityID) : mesh(mesh), worldPosition(worldPosition), globalIndex(index), entityID(entityID) {}
	};

	class BVHTree
	{
	public:
		BVHTree(unsigned int maxObjectsPerNode = 3u);
		~BVHTree();

		void BuildTree(const std::vector<std::pair<std::pair<glm::vec3, unsigned int>, Mesh*>>& unsortedObjects);
		BVHNode* GetRootNode() { return rootNode; }
		const unsigned int GetNodeCount() const { return nodeCount; }
		const std::vector<BVHObject>& GetGlobalObjects() const { return globalObjects; }
	private:
		std::vector<BVHObject> globalObjects;

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

		BVHNode* rootNode;
		unsigned int maxObjectsPerNode;
		unsigned int nodeCount;

		bool CreateNodeAABB(const std::vector<unsigned int>& indices, AABBPoints& out_aabb) {
			if (indices.size() > 0) {
				const glm::vec3& position = globalObjects[indices[0]].worldPosition;
				const AABBPoints& geoBounds = globalObjects[indices[0]].mesh->GetGeometryAABB();

				float minX = position.x + geoBounds.minX;
				float minY = position.y + geoBounds.minY;
				float minZ = position.z + geoBounds.minZ;
				float maxX = position.x + geoBounds.maxX;
				float maxY = position.y + geoBounds.maxY;
				float maxZ = position.z + geoBounds.maxZ;

				for (int i = 1; i < indices.size(); i++) {
					const AABBPoints& geoBounds = globalObjects[indices[i]].mesh->GetGeometryAABB();
					const glm::vec3& position = globalObjects[indices[i]].worldPosition;

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
		bool SplitNode(unsigned int axisIndex, float axisExtent, const std::vector<unsigned int>& unsortedIndices, const AABBPoints& parentBounds, BVHNode* parentNode) {
			AABBPoints leftAABB, rightAABB;

			// Sort meshes from smallest position to biggest position on biggest extent axis
			std::vector<unsigned int> sortedIndices = SortMeshesOnAxis(unsortedIndices, axisIndex);

			// Split meshes into left and right
			// --------------------------------
			glm::vec3 parentMin = glm::vec3(parentBounds.minX, parentBounds.minY, parentBounds.minZ);
			float halfExtent = parentMin[axisIndex] + axisExtent * 0.5f;

			// Find split index
			unsigned int splitIndex = GetSplitIndex(sortedIndices, halfExtent, axisIndex);

			if (splitIndex == 0) {
				// Split axis didn't work
				return false;
			}

			// Create AABB for left and right nodes
			std::vector<unsigned int> leftSplit(sortedIndices.begin(), sortedIndices.begin() + splitIndex);
			std::vector<unsigned int> rightSplit(sortedIndices.begin() + splitIndex, sortedIndices.end());

			bool leftSuccess = CreateNodeAABB(leftSplit, leftAABB);
			bool rightSuccess = CreateNodeAABB(rightSplit, rightAABB);

			if (leftSuccess) {
				parentNode->SetLeftChild(BuildNode(parentNode, leftSplit, leftAABB));
			}

			if (rightSuccess) {
				parentNode->SetRightChild(BuildNode(parentNode, rightSplit, rightAABB));
			}

			return true;
		}
		std::vector<unsigned int> SortMeshesOnAxis(const std::vector<unsigned int>& unsortedIndices, unsigned int axisIndex) {
			std::vector<unsigned int> sorted = unsortedIndices;

			switch (axisIndex) {
			case 0:
				QuickSortIndicesXAxis(sorted, 0, sorted.size() - 1);
				break;
			case 1:
				QuickSortIndicesYAxis(sorted, 0, sorted.size() - 1);
				break;
			case 2:
				QuickSortIndicesZAxis(sorted, 0, sorted.size() - 1);
				break;
			default:
				break;
			}

			return sorted;
		}
		unsigned int GetSplitIndex(const std::vector<unsigned int>& sortedIndices, float halfExtent, unsigned int axisIndex) {
			if (sortedIndices.size() == 0) {
				return 0;
			}

			for (int i = 0; i < sortedIndices.size(); i++) {
				const BVHObject& bvhObject = globalObjects[sortedIndices[i]];
				glm::vec3 position = bvhObject.worldPosition;
				if (position[axisIndex] >= halfExtent) {
					return i;
				}
			}
			return sortedIndices.size() - 1;
		}
		BVHNode* BuildNode(BVHNode* parent, const std::vector<unsigned int>& localIndices, const AABBPoints aabb) {
			BVHNode* node = nullptr;
			if (localIndices.size() > 0u) {
				nodeCount++;
				node = new BVHNode(parent);
				node->SetBoundingBox(aabb);
				node->SetObjectIndices(localIndices);
				node->SetIsLeaf(false);

				// Check number of objects in this node
				if (localIndices.size() <= maxObjectsPerNode) {
					node->SetIsLeaf(true);
				}
				else {
					// Split node
					bool success = BuildTreeRecursive(localIndices, node);
					if (!success) {
						node->SetIsLeaf(true);
					}
				}
			}
			return node;
		}

		// Returns false if parent node could not be split on any axis
		bool BuildTreeRecursive(const std::vector<unsigned int>& indicesUnsorted, BVHNode* parentNode);

		void QuickSortIndicesXAxis(std::vector<unsigned int>& indicesUnsorted, const int low, const int high) {
			if (low < high) {
				int partition = QuickSortSplitXCompare(indicesUnsorted, low, high);

				QuickSortIndicesXAxis(indicesUnsorted, low, partition - 1);
				QuickSortIndicesXAxis(indicesUnsorted, partition + 1, high);
			}
		}
		void QuickSortIndicesYAxis(std::vector<unsigned int>& indicesUnsorted, const int low, const int high) {
			if (low < high) {
				int partition = QuickSortSplitYCompare(indicesUnsorted, low, high);

				QuickSortIndicesYAxis(indicesUnsorted, low, partition - 1);
				QuickSortIndicesYAxis(indicesUnsorted, partition + 1, high);
			}
		}
		void QuickSortIndicesZAxis(std::vector<unsigned int>& indicesUnsorted, const int low, const int high) {
			if (low < high) {
				int partition = QuickSortSplitZCompare(indicesUnsorted, low, high);

				QuickSortIndicesZAxis(indicesUnsorted, low, partition - 1);
				QuickSortIndicesZAxis(indicesUnsorted, partition + 1, high);
			}
		}

		int QuickSortSplitXCompare(std::vector<unsigned int>& indices, const int low, const int high) {
			unsigned int pivot = indices[high];
			const BVHObject& pivotObject = globalObjects[pivot];

			int i = low - 1;
			for (int j = low; j <= high - 1; j++) {
				const BVHObject& jObject = globalObjects[indices[j]];
				if (ComparePositionX(jObject, pivotObject)) {
					i++;
					std::swap(indices[i], indices[j]);
				}
			}

			std::swap(indices[i + 1], indices[high]);
			return i + 1;
		}
		int QuickSortSplitYCompare(std::vector<unsigned int>& indices, const int low, const int high) {
			unsigned int pivot = indices[high];
			const BVHObject& pivotObject = globalObjects[pivot];

			int i = low - 1;
			for (int j = low; j <= high - 1; j++) {
				const BVHObject& jObject = globalObjects[indices[j]];
				if (ComparePositionY(jObject, pivotObject)) {
					i++;
					std::swap(indices[i], indices[j]);
				}
			}

			std::swap(indices[i + 1], indices[high]);
			return i + 1;
		}
		int QuickSortSplitZCompare(std::vector<unsigned int>& indices, const int low, const int high) {
			unsigned int pivot = indices[high];
			const BVHObject& pivotObject = globalObjects[pivot];

			int i = low - 1;
			for (int j = low; j <= high - 1; j++) {
				const BVHObject& jObject = globalObjects[indices[j]];
				if (ComparePositionZ(jObject, pivotObject)) {
					i++;
					std::swap(indices[i], indices[j]);
				}
			}

			std::swap(indices[i + 1], indices[high]);
			return i + 1;
		}

		bool ComparePositionX(const BVHObject& one, const BVHObject& two) {
			return one.worldPosition.x < two.worldPosition.x;
		}
		bool ComparePositionY(const BVHObject& one, const BVHObject& two) {
			return one.worldPosition.y < two.worldPosition.y;
		}
		bool ComparePositionZ(const BVHObject& one, const BVHObject& two) {
			return one.worldPosition.z < two.worldPosition.z;
		}
	};
}