#pragma once
#include "ComponentCollisionAABB.h"
#include "Mesh.h"
namespace Engine {
	class BVHNode
	{
	public:
		BVHNode(BVHNode* parent = nullptr);
		~BVHNode();

		void SetParent(BVHNode* newParent) { this->parentNode = newParent; }
		void SetLeftChild(BVHNode* newChild) { this->leftChildNode = newChild; }
		void SetRightChild(BVHNode* newChild) { this->rightChildNode = newChild; }

		void SetBoundingBox(const AABBPoints newBox) { this->boundingBox = newBox; }

		const BVHNode* GetParent() const { return parentNode; }
		const BVHNode* GetLeftChild() const { return leftChildNode; }
		const BVHNode* GetRightChild() const { return rightChildNode; }

		const bool IsLeaf() const { return isLeaf; }
		void SetIsLeaf(const bool isLeaf) { this->isLeaf = isLeaf; }

		const std::vector<std::pair<glm::vec3, Mesh*>>& GetObjects() const { return objects; }
		void SetObjects(std::vector<std::pair<glm::vec3, Mesh*>> newObjects) { objects = newObjects; }
		
		void SetObjectIndices(std::vector<unsigned int> indices) { this->globalObjectIndices = indices; }
		const std::vector<unsigned int>& GetGlobalObjectIndices() const { return globalObjectIndices; }

		const AABBPoints& GetBoundingBox() const { return boundingBox; }
	private:
		AABBPoints boundingBox;

		BVHNode* parentNode;
		BVHNode* leftChildNode;
		BVHNode* rightChildNode;

		std::vector<std::pair<glm::vec3, Mesh*>> objects;
		std::vector<unsigned int> globalObjectIndices;

		bool isLeaf;
	};
}