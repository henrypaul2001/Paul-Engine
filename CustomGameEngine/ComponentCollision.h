#pragma once
#include "Component.h"
#include <unordered_map>
#include "Entity.h"
namespace Engine {
	class ComponentCollision : public Component
	{
	protected:
		std::unordered_map<Entity*, std::string> EntitiesCheckedThisFrame;
	public:
		virtual ComponentTypes ComponentType() override = 0;
		virtual void Close() override = 0;
		void ClearEntitiesCheckedThisFrame() { EntitiesCheckedThisFrame.clear(); }
		void AddToEntitiesCheckedThisFrame(Entity* e) { EntitiesCheckedThisFrame[e] = e->Name(); }

		bool useDefaultCollisionResponse;

		bool HasEntityAlreadyBeenChecked(Entity* e) { return EntitiesCheckedThisFrame.find(e) != EntitiesCheckedThisFrame.end(); }
	};
}