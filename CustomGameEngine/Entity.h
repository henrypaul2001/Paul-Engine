#pragma once
#include <string>
#include <vector>
#include "Component.h"
namespace Engine
{
	class Entity
	{
	private:
		std::string name;
		std::vector<Component*> componentList;
		ComponentTypes mask;

	public:
		Entity(std::string name);
		~Entity();

		std::string& Name() { return name; }
		ComponentTypes& Mask() { return mask; }
		std::vector<Component*>& Components() { return componentList; }

		void AddComponent(Component* component);
		void Close();
	};
}

