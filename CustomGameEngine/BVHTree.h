#pragma once
#include "BVHNode.h"
#include "Mesh.h"
namespace Engine {
	class BVHTree
	{
	public:
		BVHTree();
		~BVHTree();

		void BuildTree(const std::vector<std::pair<const glm::vec3, Mesh*>>& unsortedObjects);

		BVHNode* GetRootNode() { return rootNode; }
	private:
		BVHNode* rootNode;
	};
}