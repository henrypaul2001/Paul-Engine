#pragma once
#include "SparseSet.h"
#include <vector>
#include "EntityNew.h"
#include <unordered_map>

namespace Engine
{
	class EntityManagerNew
	{
	public:
		EntityManagerNew() : entities(0) {}
		~EntityManagerNew() {}

		// Create and return a new entity. If name already exists, a unique number will be appended
		EntityNew* New(const std::string& name) {
			// Check for existing name
			std::string entityName = name;
			unsigned int duplicateCount = 0;
			while (name_to_ID.find(entityName) != name_to_ID.end()) {
				duplicateCount++;
				entityName = name + " (" + std::to_string(duplicateCount) + ")";
			}

			// Create entity
			const unsigned int entityID = entities.SparseSize();
			EntityNew entity = EntityNew(entityName, entityID);
			entities.Add(entityID, entity);
			name_to_ID[entityName] = entityID;
			return entities.GetPtr(entityID);
		}

		// Find an entity by name. Returns nullptr if entity does not exist
		const EntityNew* Find(const std::string& name) const {
			std::unordered_map<std::string, unsigned int>::const_iterator it = name_to_ID.find(name);
			if (it != name_to_ID.end()) { return entities.GetPtr(it->second); }
			else { return nullptr; }
		}
		// Find entity by id. Returns nullptr if entity does not exist with this id
		const EntityNew* Find(const unsigned int id) const {
			return entities.GetPtr(id);
		}
		// Find an entity by name. Returns nullptr if entity does not exist
		EntityNew* Find(const std::string& name) {
			std::unordered_map<std::string, unsigned int>::iterator it = name_to_ID.find(name);
			if (it != name_to_ID.end()) { return entities.GetPtr(it->second); }
			else { return nullptr; }
		}
		// Find entity by id. Returns nullptr if entity does not exist with this id
		EntityNew* Find(const unsigned int id) {
			return entities.GetPtr(id);
		}

		// Delete entity by reference. Returns false if entity does not exist in manager
		bool Delete(EntityNew& entity) { return Delete(entity.ID()); }

		// Delete entity by ID. Returns false if ID does not exist in manager
		bool Delete(const unsigned int entityID) {
			if (!entities.ValidateIndex(entityID)) { return false; }

			// Remove name from map
			const std::string entityName = entities.GetRef(entityID).Name();
			name_to_ID.erase(entityName);

			return entities.Delete(entityID);
		}

	private:
		SparseSet<EntityNew> entities;
		std::vector<ISparseSet> component_pools;

		std::unordered_map<std::string, unsigned int> name_to_ID;
	};

	// A potential way to fill empty entity slots when adding a new entity after deleting another one. Have a stack of integers that represent the most recently removed entity index
	// When adding an entity, add them at the index at the top of the stack and pop the stack
}