#pragma once
#include "ComponentCollisionAABB.h"
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

		const AABBPoints& GetBoundingBox() const { return boundingBox; }
	private:
		AABBPoints boundingBox;

		BVHNode* parentNode;
		BVHNode* leftChildNode;
		BVHNode* rightChildNode;
	};
}