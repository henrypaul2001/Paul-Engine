#include "pepch.h"
#include "Prefab.h"
#include "PaulEngine/Scene/Components.h"

namespace PaulEngine
{
	Prefab::Prefab() : m_PrefabScene(CreateRef<Scene>()) {}

	Entity Prefab::Instantiate(Scene* targetScene)
	{
		if (targetScene) {
			Ref<Scene> copy = Scene::Copy(m_PrefabScene);
			std::vector<Entity> roots = copy->GetRootEntities();
			const std::string prefabName = copy->GetName();
			Entity prefabRoot = copy->CreateEntity(prefabName + "_Root");
			ComponentPrefabSource& prefabComponent = prefabRoot.AddComponent<ComponentPrefabSource>();
			prefabComponent.PrefabHandle = Handle;
			for (Entity e : roots)
			{
				ComponentTransform::SetParent(e, prefabRoot);
			}
			return prefabRoot.CopyToScene(targetScene);
		}
		return Entity();
	}

	Prefab Prefab::CreateFromEntity(Entity srcEntity)
	{
		Prefab prefab;
		std::string prefabName = srcEntity.Tag();
		prefab.m_PrefabScene->m_Name = prefabName;
		Entity copiedEntity = srcEntity.CopyToScene(prefab.m_PrefabScene.get());
		ComponentTransform::SetParent(copiedEntity, Entity());
		return prefab;
	}
}