#pragma once
#include "BVHNode.h"
namespace Engine {
	class BVHTree
	{
	public:
		BVHTree();
		~BVHTree();

	private:
		BVHNode rootNode;
	};
}