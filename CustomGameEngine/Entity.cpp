#include "Entity.h"
namespace Engine
{
	Entity::Entity(std::string name) {
		this->name = name;
	}

	Entity::~Entity() {
		delete &name;
		delete& componentList;
		delete& mask;
	}

	void Entity::AddComponent(Component* component) {
		_ASSERT(&component != nullptr, "Component cannot be null");

		componentList.push_back(component);
		mask |= component->ComponentType();
	}

	void Entity::Close() {
		for (Component* c : componentList) {
			c->Close();
		}
	}
}