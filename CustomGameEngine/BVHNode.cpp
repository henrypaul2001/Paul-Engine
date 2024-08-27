#include "BVHNode.h"
namespace Engine {
	BVHNode::BVHNode(BVHNode* parent)
	{
		this->parentNode = parent;
		this->leftChildNode = nullptr;
		this->rightChildNode = nullptr;
		isLeaf = false;
	}

	BVHNode::~BVHNode()
	{
		if (leftChildNode) { delete leftChildNode; }
		if (rightChildNode) { delete rightChildNode; }
	}
}