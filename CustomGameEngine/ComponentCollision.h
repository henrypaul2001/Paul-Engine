#pragma once
#include "Component.h"
#include <unordered_map>
#include "Entity.h"
namespace Engine {
	class ComponentCollision : public Component
	{
	protected:
		std::unordered_map<Entity*, std::string> EntitiesCheckedThisFrame;
		std::unordered_map<Entity*, std::string> EntitiesCollidingWith;

		bool isMovedByCollisions;
	public:
		virtual ComponentTypes ComponentType() override = 0;
		virtual void Close() override = 0;
		
		void ClearEntitiesCheckedThisFrame() { EntitiesCheckedThisFrame.clear(); }
		void AddToEntitiesCheckedThisFrame(Entity* e) { EntitiesCheckedThisFrame[e] = e->Name(); }

		bool useDefaultCollisionResponse;

		bool HasEntityAlreadyBeenChecked(Entity* e) { return EntitiesCheckedThisFrame.find(e) != EntitiesCheckedThisFrame.end(); }

		bool IsMovedByCollisions() { return isMovedByCollisions; }
		void IsMovedByCollisions(bool isMoveable) { isMovedByCollisions = isMoveable; }

		std::unordered_map<Entity*, std::string> Collisions() { return EntitiesCollidingWith; }
		bool IsCollidingWithEntity(Entity* e) { return EntitiesCollidingWith.find(e) != EntitiesCollidingWith.end(); }
		void AddToCollisions(Entity* e) { EntitiesCollidingWith[e] = e->Name(); }
		void RemoveFromCollisions(Entity* e) { EntitiesCollidingWith.erase(e); }
	};
}