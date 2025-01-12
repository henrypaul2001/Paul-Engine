#pragma once
#include "SparseSet.h"
#include <vector>
#include "EntityNew.h"
#include <unordered_map>
#include <typeindex>

namespace Engine
{
	class EntityManagerNew
	{
	public:
		EntityManagerNew() : entities(0) {}
		~EntityManagerNew() {}

		// Entity functions
		// ----------------
		
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
			unsigned int entityID = entities.SparseSize();
			if (empty_slots.size() > 0) {
				// Use most recently deleted id
				entityID = empty_slots.top();
				empty_slots.pop();
			}

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
		// Delete entity by name. Returns false if name does not exist in manager
		bool Delete(const std::string& name) {
			std::unordered_map<std::string, unsigned int>::iterator it = name_to_ID.find(name);
			if (it != name_to_ID.end()) { return Delete(it->second); }
			return false;
		}
		// Delete entity by ID. Returns false if ID does not exist in manager
		bool Delete(const unsigned int entityID) {
			if (!entities.ValidateIndex(entityID)) { return false; }

			// Remove name from map
			const std::string entityName = entities.GetRef(entityID).Name();
			name_to_ID.erase(entityName);

			bool success = entities.Delete(entityID);
			if (success) {
				empty_slots.push(entityID);
				
				// Delete component entries
				// for each pool
				// check mask[i] == 1
				// pool[i]->delete(id)
			}
			return success;
		}

		// Component functions
		// -------------------

		template <typename T>
		void RegisterComponentType() {
			AddComponentBitPosition<T>();
			component_pools.push_back(std::make_unique<SparseSet<T>>());
		}
	private:

		template <typename T>
		const int GetComponentBitPosition() {
			std::type_index type = std::type_index(typeid(T));
			std::unordered_map<std::type_index, unsigned int>::iterator it = component_bit_positions.find(type);
			if (it != component_bit_positions.end()) {
				// Component index found
				return it->second;
			}
			else {
				// Component not found. May not have been registered with ecs yet
				return -1;
			}
		}

		template <typename T>
		void AddComponentBitPosition() {
			std::type_index type = std::type_index(typeid(T));

			// Check if already registered
			if (GetComponentBitPosition<T>() == -1) {
				return;
			}

			component_bit_positions[type] = component_pools.size();
		}

		SparseSet<EntityNew> entities;
		std::stack<unsigned int> empty_slots;
		std::vector<std::unique_ptr<ISparseSet>> component_pools;

		std::unordered_map<std::string, unsigned int> name_to_ID;
		
		std::unordered_map<std::type_index, unsigned int> component_bit_positions;
	};
}