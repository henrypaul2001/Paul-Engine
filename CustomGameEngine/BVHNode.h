#pragma once
#include "ComponentCollisionAABB.h"
namespace Engine {
	class BVHNode
	{
	public:
		BVHNode();
		~BVHNode();

	private:
		AABBPoints boundingBox;

		std::vector<BVHNode*> childNodes;
	};
}