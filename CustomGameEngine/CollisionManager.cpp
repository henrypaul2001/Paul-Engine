#include "CollisionManager.h"
#include "SystemBuildMeshList.h"
namespace Engine {
	CollisionManager::CollisionManager()
	{
		bvhTree = new BVHTree();
	}

	CollisionManager::~CollisionManager()
	{
		delete bvhTree;
	}

	void CollisionManager::ConstructBVHTree()
	{
		SCOPE_TIMER("CollisionManager::ConstructBVHTree");
		if (bvhTree) {
			delete bvhTree;
			bvhTree = new BVHTree();
		}

		bvhTree->BuildTree(SystemBuildMeshList::MeshList());
	}
}