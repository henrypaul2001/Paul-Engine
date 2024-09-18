#include "CollisionManager.h"
namespace Engine {
	CollisionManager::CollisionManager()
	{
		bvhTree = new BVHTree();
	}

	CollisionManager::~CollisionManager()
	{
		delete bvhTree;
	}

	void CollisionManager::ConstructBVHTree(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("CollisionManager::ConstructBVHTree");
		if (bvhTree) {
			delete bvhTree;
			bvhTree = new BVHTree();
		}
		std::vector<std::pair<glm::vec3, Mesh*>> centrePosAndMeshesList;
		centrePosAndMeshesList.reserve(entityList.size());

		{
			SCOPE_TIMER("CollisionManager::ConstructBVHTree::CreateMeshList");
			for (Entity* e : entityList) {
				if (e->ContainsComponents(COMPONENT_GEOMETRY)) {
					glm::vec3 pos = e->GetTransformComponent()->GetWorldPosition(); // temporarily use entity pos as AABB centre

					std::vector<Mesh*> meshList = e->GetGeometryComponent()->GetModel()->meshes;
					for (Mesh* m : meshList) {
						centrePosAndMeshesList.push_back(std::make_pair(pos, m));
					}
				}
			}
		}

		bvhTree->BuildTree(centrePosAndMeshesList);
	}
}