#pragma once
#include <vector>
#include <string>
#include "Entity.h"
namespace Engine
{
	class EntityManager
	{
	private:
		std::vector<Entity> entityList;

	public:
		EntityManager();
		~EntityManager();

		void AddEntity(Entity& entity);
		Entity* FindEntity(std::string name);
		void Close();

		std::vector<Entity>& Entities() { return entityList; }
	};
}

