#pragma once
#include <vector>
#include "EntityNew.h"
#include "SparseSet.h"
#include <unordered_map>

namespace Engine
{
	class EntityManagerNew
	{
	public:
		EntityManagerNew() : num_entities(0) {}
		~EntityManagerNew() {}

		// Create a new entity, assign ID based on current end point of sparse set, check unique name, return pointer to new entity
		EntityNew* New(const std::string& name) {
			// Check for existing name
			std::string entityName = name;
			unsigned int duplicateCount = 0;
			while (name_to_ID.find(entityName) != name_to_ID.end()) {
				duplicateCount++;
				entityName = name + " (" + std::to_string(duplicateCount) + ")";
			}

			// Create entity
			const unsigned int entityID = num_entities;
			EntityNew entity = EntityNew(entityName, entityID);
			entities.Add(entityID, entity);
			name_to_ID[entityName] = entityID;
			num_entities++;
			return entities.GetPtr(entityID);
		}

	private:
		SparseSet<EntityNew> entities;
		std::vector<ISparseSet> component_pools;

		std::unordered_map<std::string, unsigned int> name_to_ID;

		unsigned int num_entities;
	};

	// A potential way to fill empty entity slots when adding a new entity after deleting another one. Have a stack of integers that represent the most recently removed entity index
	// When adding an entity, add them at the index at the top of the stack and pop the stack
}