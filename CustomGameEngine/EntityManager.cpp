#include "EntityManager.h"
namespace Engine
{
	EntityManager::EntityManager() {}

	EntityManager::~EntityManager()
	{
		Close();
	}

	void EntityManager::AddEntity(Entity* entity)
	{
		Entity* result = FindEntity(entity->Name());
		_ASSERT(result == nullptr, "Entity '" + entity.Name() + "' already exists");
		entityList.push_back(entity);
	}

	Entity* EntityManager::FindEntity(std::string name)
	{
		for (Entity* e : entityList) {
			if (e->Name() == name) {
				return e;
			}
		}

		return nullptr;
	}

	void EntityManager::Close()
	{
		for (Entity* e : entityList) {
			e->Close();
			delete e;
		}
	}
}