#include "BVHNode.h"
namespace Engine {
	BVHNode::BVHNode(BVHNode* parent)
	{
		this->parentNode = parent;
		this->leftChildNode = nullptr;
		this->rightChildNode = nullptr;
	}

	BVHNode::~BVHNode()
	{
		if (leftChildNode) { delete leftChildNode; }
		if (rightChildNode) { delete rightChildNode; }
	}
}