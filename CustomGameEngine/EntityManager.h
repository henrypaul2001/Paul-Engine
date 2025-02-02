#pragma once
#include "ComponentTransform.h"
#include "SparseSet.h"
#include <vector>
#include "Entity.h"
#include <unordered_map>
#include <typeindex>
#include "View.h"
#include <memory>
#include <stack>
#include <concepts>

namespace Engine
{
	template <typename TComponent>
	concept NotComponentTransform = !std::is_same_v<TComponent, ComponentTransform>;
	static constexpr unsigned int INVALID_ID = std::numeric_limits<unsigned int>::max();

	class EntityManager
	{
	public:
		EntityManager(const unsigned int init_size = 10) : entities(init_size) {
			for (unsigned int i = 0; i < init_size; i++) {
				empty_slots.push(i);
			}
		}
		~EntityManager() {}

		// Entity functions
		// ----------------

		// Create and return a new entity with a default transform component. If name already exists, a unique number will be appended
		Entity* New(const std::string& name) {
			Entity* entity = NewWithoutDefault(name);

			AddComponent(entity->id, ComponentTransform(this, 0.0f, 0.0f, 0.0f));
			GetComponent<ComponentTransform>(entity->id)->ownerID = entity->id;
			return entity;
		}

		// Clone an entity by ID. Returns pointer to new entity. Returns nullptr if ID doesn't exist
		Entity* Clone(const unsigned int entityID) {
			return Clone(entities.GetRef(entityID));
		}
		// Clone an entity by name. Returns pointer to new entity. Returns nullptr if name doesn't exist
		Entity* Clone(const std::string& name) { return Clone(*Find(name)); }

		// Find an entity by name. Returns nullptr if entity does not exist
		const Entity* Find(const std::string& name) const {
			std::unordered_map<std::string, unsigned int>::const_iterator it = name_to_ID.find(name);
			if (it != name_to_ID.end()) { return entities.GetPtr(it->second); }
			else { return nullptr; }
		}
		// Find entity by id. Returns nullptr if entity does not exist with this id
		const Entity* Find(const unsigned int id) const {
			return entities.GetPtr(id);
		}
		// Find an entity by name. Returns nullptr if entity does not exist
		Entity* Find(const std::string& name) {
			std::unordered_map<std::string, unsigned int>::iterator it = name_to_ID.find(name);
			if (it != name_to_ID.end()) { return entities.GetPtr(it->second); }
			else { return nullptr; }
		}
		// Find entity by id. Returns nullptr if entity does not exist with this id
		Entity* Find(const unsigned int id) {
			return entities.GetPtr(id);
		}

		// Delete entity by reference. Returns false if entity does not exist in manager
		bool Delete(Entity& entity) { return Delete(entity.ID()); }
		// Delete entity by name. Returns false if name does not exist in manager
		bool Delete(const std::string& name) {
			std::unordered_map<std::string, unsigned int>::iterator it = name_to_ID.find(name);
			if (it != name_to_ID.end()) { return Delete(it->second); }
			return false;
		}
		// Delete entity by ID. Returns false if ID does not exist in manager
		bool Delete(const unsigned int entityID) {
			if (!entities.ValidateIndex(entityID)) { return false; }
			Entity& entity = entities.GetRef(entityID);
			const std::string entityName = entity.Name();
			std::bitset<MAX_COMPONENTS> mask = entity.component_mask;

			bool success = entities.Delete(entityID);
			if (success) {
				// Remove name from map
				name_to_ID.erase(entityName);

				empty_slots.push(entityID);

				// Delete component entries
				for (int i = 0; i < component_pools.size(); i++) {
					component_pools[i].get()->Delete(entityID);
				}
			}
			return success;
		}

		// Component functions
		// -------------------

		template <typename TComponent>
		bool HasComponent(const unsigned int entityID) const {
			const std::bitset<MAX_COMPONENTS>& mask = entities.GetRef(entityID).component_mask;
			int index = GetComponentBitPosition<TComponent>();
			if (index != -1) { return mask[index]; }
			else { return false; }
		}
		template <typename TComponent>
		bool HasComponent(const Entity& entity) const {
			const std::bitset<MAX_COMPONENTS>& mask = entity.component_mask;
			const int index = GetComponentBitPosition<TComponent>();
			if (index != -1) { return mask[index]; }
			else { return false; }
		}
		template <typename TComponent>
		bool HasComponent(const std::string& entityName) const {
			const std::bitset<MAX_COMPONENTS>& mask = Find(entityName).component_mask;
			const int index = GetComponentBitPosition<TComponent>();
			if (index != -1) { return mask[index]; }
			else { return false; }
		}

		template <typename TComponent>
		bool AddComponent(const unsigned int entityID, const TComponent& component) {
			if (!HasComponent<TComponent>(entityID)) {
				int bitPosition = RegisterComponentType<TComponent>();
				if (bitPosition == -1) { return false; }

				SparseSet<TComponent>* component_pool = GetComponentPoolPtrCasted<TComponent>();
				component_pool->Add(entityID, component);

				entities.GetPtr(entityID)->component_mask.set(GetComponentBitPosition<TComponent>());
				return true;
			}
			else { return false; }
		}

		// Remove a component or multiple components from a specified entity ID.
		// Attempting to remove ComponentTransform will cause a build error
		template <typename... TComponents>
		void RemoveComponent(const unsigned int entityID) requires (NotComponentTransform<TComponents>&&...) {
			(RemoveComponentPrivate<TComponents>(entityID), ...);
		}

		// Return pointer to component type, returns nullptr if entity does not own component
		template <typename TComponent>
		TComponent* GetComponent(const unsigned int entityID) {
			if (HasComponent<TComponent>(entityID)) {
				SparseSet<TComponent>* pool = GetComponentPoolPtrCasted<TComponent>();
				return pool->GetPtr(entityID);
			}
			else { return nullptr; }
		}

		// Register component and return bit position
		// Returns -1 if component couldn't be registered
		template <typename TComponent>
		int RegisterComponentType() {
			return GetAddComponentBitPosition<TComponent>();
		}

		// Generate a mask of components from templated types
		template <typename... TComponents>
		std::bitset<MAX_COMPONENTS> CreateMask() {
			std::bitset<MAX_COMPONENTS> mask;
			(mask.set(GetAddComponentBitPosition<TComponents>(), true), ...);
			return mask;
		}

		template <typename... TComponents>
		View<TComponents...> View() {
			return { { GetComponentPoolPtr<TComponents>()... } };
		}

		const unsigned int NumEntities() const { return entities.DenseSize(); }

	private:
		// Clone an entity by reference. Returns pointer to new entity. Returns nullptr if entity doesn't exist in ECS
		Entity* Clone(const Entity& entity) {
			// Clone entity
			const unsigned int old_id = entity.id;
			const std::string& old_name = entity.name;
			const std::bitset<MAX_COMPONENTS> old_mask = entity.component_mask;

			Entity* newEntity = NewWithoutDefault(old_name);
			const unsigned int new_id = newEntity->ID();

			// Clone transform component
			AddComponent(new_id, GetComponent<ComponentTransform>(old_id)->Clone());
			ComponentTransform* transform = GetComponent<ComponentTransform>(new_id);
			transform->ownerID = new_id;
			for (unsigned int child : transform->childrenIDs) {
				GetComponent<ComponentTransform>(child)->parentID = new_id;
			}

			// Clone components
			for (int i = 1; i < component_pools.size(); i++) {
				if (old_mask[i]) {
					// Copy and add to new entity
					component_pools[i].get()->CloneElement(old_id, new_id);
				}
			}

			newEntity = Find(new_id);
			newEntity->component_mask = old_mask;

			return newEntity;
		}

		// Create new entity without default transform component (for use during entity cloning)
		Entity* NewWithoutDefault(const std::string& name) {
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

			Entity entity = Entity(entityName, entityID);
			entities.Add(entityID, entity);
			name_to_ID[entityName] = entityID;
			return entities.GetPtr(entityID);
		}

		// Get uncasted ptr to ISparseSet for component type TComponent
		template <typename TComponent>
		ISparseSet* GetComponentPoolPtr() {
			int index = GetComponentBitPosition<TComponent>();
			if (index == -1) {
				RegisterComponentType<TComponent>();
				index = GetComponentBitPosition<TComponent>();
			}

			return component_pools[index].get();
		}

		// Get casted ptr to SparseSet for component type TComponent
		template <typename TComponent>
		SparseSet<TComponent>* GetComponentPoolPtrCasted() {
			ISparseSet* ptr = GetComponentPoolPtr<TComponent>();
			return static_cast<SparseSet<TComponent>*>(ptr);
		}

		// Better optimised alternative of: if (!GetComponentBitPosition()) then { AddComponentBitPosition() }
		template <typename T>
		const int GetAddComponentBitPosition() {
			int position = GetComponentBitPosition<T>();
			if (position != -1) {
				return position;
			}
			else {
				std::type_index type = std::type_index(typeid(T));
				position = component_pools.size();
				if (position >= MAX_COMPONENTS) {
					return -1;
				}
				component_bit_positions[type] = position;
				component_pools.push_back(std::make_unique<SparseSet<T>>());
				return position;
			}
		}

		template <typename T>
		const int GetComponentBitPosition() const {
			std::type_index type = std::type_index(typeid(T));
			std::unordered_map<std::type_index, unsigned int>::const_iterator it = component_bit_positions.find(type);
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
			if (GetComponentBitPosition<T>() != -1) {
				return;
			}

			const unsigned int position = component_pools.size();
			if (position >= MAX_COMPONENTS) {
				return -1;
			}
			component_bit_positions[type] = position;
			component_pools.push_back(std::make_unique<SparseSet<T>>());
		}

		template <typename TComponent>
		void RemoveComponentPrivate(const unsigned int entityID) {
			Entity& entity = entities.GetRef(entityID);
			if (HasComponent<TComponent>(entity)) {
				const int position = GetComponentBitPosition<TComponent>();
				SparseSet<TComponent>* pool = GetComponentPoolPtrCasted<TComponent>();
				pool->Delete(entityID);
				entity.component_mask.set(position, false);
			}
		}

		SparseSet<Entity> entities;
		std::stack<unsigned int> empty_slots;
		std::vector<std::unique_ptr<ISparseSet>> component_pools;

		std::unordered_map<std::string, unsigned int> name_to_ID;
		
		std::unordered_map<std::type_index, unsigned int> component_bit_positions;
	};
}