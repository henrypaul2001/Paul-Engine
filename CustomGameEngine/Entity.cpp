#include "Entity.h"
#include "EntityManager.h"
namespace Engine
{
	Entity::Entity(const Entity& old_entity)
	{
		this->entityManager = old_entity.entityManager;
		this->name = old_entity.name;
		this->mask = COMPONENT_NONE;

		// Copy components
		for (unsigned int i = 0; i < GetComponentContainerSize(); i++) {
			Component* oldComponent = old_entity.components[i];
			if (oldComponent != nullptr) {
				Component* newComponent = oldComponent->Copy();
				newComponent->SetOwner(this);
				if (newComponent->ComponentType() == COMPONENT_TRANSFORM) {
					for (Entity* child : dynamic_cast<ComponentTransform*>(newComponent)->GetChildren()) {
						child->GetTransformComponent()->SetParent(this);
					}
				}
				AddComponent(newComponent);
			}
			else {
				components[i] = nullptr;
			}
		}
	}

	Entity::Entity(std::string name) {
		this->name = name;
		mask = COMPONENT_NONE;
	}

	Entity::~Entity() {
		Close();
	}

	Component* Entity::GetComponent(ComponentTypes type)
	{
		unsigned int index = GetComponentIndex(type);
		return components[index];
	}

	Entity* Entity::Clone()
	{
		Entity* cloned = new Entity(*this);
		entityManager->AddEntity(cloned);
		return cloned;
	}

	EntityManager* Entity::GetEntityManager()
	{
		return entityManager;
	}

	void Entity::SetEntityManager(EntityManager* manager)
	{
		this->entityManager = manager;
	}

	bool Entity::ContainsComponents(const ComponentTypes MASK)
	{
		return ((mask & MASK) == MASK);
	}

	Component* Entity::RemoveGetComponent(int componentIndex)
	{
		Component* result = nullptr;
		if (componentIndex < GetComponentContainerSize()) {
			result = components[componentIndex];
			mask = mask & ~result->ComponentType();
			components[componentIndex] = nullptr;
		}
		return result;
	}

	Component* Entity::RemoveGetComponent(ComponentTypes type)
	{
		unsigned int index = GetComponentIndex(type);

		Component* result = components[index];
		mask = mask & ~result->ComponentType();
		components[index] = nullptr;

		return nullptr;
	}

	void Entity::RemoveComponent(Component* component)
	{
		unsigned int index = GetComponentIndex(component->ComponentType());
		mask = mask & ~component->ComponentType();
		components[index] = nullptr;
	}

	void Entity::RemoveAndDeleteComponent(const ComponentTypes type)
	{
		unsigned int index = GetComponentIndex(type);
		Component* component = components[index];
		mask = mask & ~component->ComponentType();
		delete component;
		components[index] = nullptr;
	}

	void Entity::RemoveAndDeleteComponent(const unsigned int componentIndex)
	{
		if (componentIndex < GetComponentContainerSize()) {
			Component* component = components[componentIndex];
			mask = mask & ~component->ComponentType();
			delete component;
			components[componentIndex] = nullptr;
		}
	}

	void Entity::AddComponent(Component* component) {
		_ASSERT(&component != nullptr, "Component cannot be null");

		components[GetComponentIndex(component->ComponentType())] = component;

		mask = mask | component->ComponentType();

		component->SetOwner(this);
		component->OnAddedToEntity();
	}

	void Entity::Close() {
		for (unsigned int i = 0; i < GetComponentContainerSize(); i++) {
			if (components[i]) {
				components[i]->Close();
				delete components[i];
			}
		}
	}
}