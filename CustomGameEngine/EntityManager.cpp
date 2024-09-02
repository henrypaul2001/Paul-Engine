#include "EntityManager.h"
#include "ComponentTransform.h"
#include "ComponentLight.h"
#include "LightManager.h"
namespace Engine
{
	EntityManager::EntityManager() {
		LightManager::GetInstance()->ResetScene();
	}

	EntityManager::~EntityManager()
	{
		Close();
	}

	void EntityManager::AddEntity(Entity* entity)
	{
		Entity* result = FindEntity(entity->Name());
		
		std::string name = entity->Name();
		int number = 0;
		while (result != nullptr) {

			number++;
			entity->Name() = std::string(name + " (" + std::to_string(number) + ")");
			result = FindEntity(entity->Name());
		}

		if (number > 0) {
			std::cout << "Entity '" << name << "' already exists. Adding unique number: " << entity->Name() << std::endl;
		}

		ComponentTransform* transform = dynamic_cast<ComponentTransform*>(entity->GetComponent(COMPONENT_TRANSFORM));
		ComponentLight* light = dynamic_cast<ComponentLight*>(entity->GetComponent(COMPONENT_LIGHT));

		_ASSERT(transform != nullptr, "Cannot create entity without transform component");

		entityList.push_back(entity);
		entity->SetEntityManager(this);

		if (light != nullptr) {
			if (light->GetLightType() == DIRECTIONAL) {
				LightManager::GetInstance()->SetDirectionalLightEntity(entity);
			}
			else {
				LightManager::GetInstance()->AddLightEntity(entity);
			}
		}
	}

	Entity* EntityManager::FindEntity(std::string name)
	{
		for (Entity* e : entityList) {
			if (e->Name() == name) {
				return e;
			}
		}

		return nullptr;
	}

	void EntityManager::Close()
	{
		for (Entity* e : entityList) {
			delete e;
		}
	}
}