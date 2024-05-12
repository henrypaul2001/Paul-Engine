#include "Entity.h"
namespace Engine
{
	Entity::Entity(std::string name) {
		this->name = name;
		mask = COMPONENT_NONE;
	}

	Entity::~Entity() {

	}

	Component* Entity::GetComponent(ComponentTypes type)
	{
		for (Component* c : componentList) {
			if (c->ComponentType() == type) {
				return c;
			}
		}
		return nullptr;
	}

	bool Entity::ContainsComponents(const ComponentTypes MASK)
	{
		return ((mask & MASK) == MASK);
	}

	Component* Entity::RemoveGetComponent(int componentIndex)
	{
		Component* result = nullptr;
		if (componentIndex < componentList.size()) {
			result = componentList[componentIndex];
			mask = mask & ~result->ComponentType();
			componentList.erase(componentList.begin() + componentIndex);
		}
		return result;
	}

	Component* Entity::RemoveGetComponent(ComponentTypes type)
	{
		for (int i = 0; i < componentList.size(); i++) {
			if (componentList[i]->ComponentType() == type) {
				Component* result = componentList[i];
				mask = mask & ~result->ComponentType();
				componentList.erase(componentList.begin() + i);
			}
		}
		return nullptr;
	}

	void Entity::RemoveComponent(Component* component)
	{
		for (int i = 0; i < componentList.size(); i++) {
			if (componentList[i] == component) {
				mask = mask & ~component->ComponentType();
				componentList.erase(componentList.begin() + i);
				return;
			}
		}
	}

	void Entity::AddComponent(Component* component) {
		_ASSERT(&component != nullptr, "Component cannot be null");

		componentList.push_back(component);
		mask = mask | component->ComponentType();

		component->SetOwner(this);
		component->OnAddedToEntity();
	}

	void Entity::Close() {
		for (Component* c : componentList) {
			c->Close();
			delete c;
		}
	}
}