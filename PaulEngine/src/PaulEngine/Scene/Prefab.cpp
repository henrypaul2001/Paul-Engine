#include "pepch.h"
#include "Prefab.h"
#include "PaulEngine/Scene/Components.h"
#include "PaulEngine/Asset/AssetManager.h"

namespace PaulEngine
{
	Prefab::Prefab() : m_PrefabScene(CreateRef<Scene>()) {}

	Entity Prefab::Instantiate(Scene* targetScene)
	{
		PE_PROFILE_FUNCTION();
		if (targetScene) {
			const std::string prefabName = m_PrefabScene->GetName();
			Entity prefabRoot = targetScene->CreateEntity(prefabName + "_Root");
			ComponentPrefabSource& prefabComponent = prefabRoot.AddComponent<ComponentPrefabSource>();
			prefabComponent.PrefabHandle = Handle;

			RefreshInstance(prefabRoot);

			return prefabRoot;
		}
		PE_CORE_ERROR("Invalid target scene for prefab instantiation");
		return Entity();
	}

	// Resets a prefab instance to its source. Will destroy all children of prefab root and recreate them, 
	// any children not found in the prefab asset will be lost
	void Prefab::RefreshInstance(Entity instance)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(instance.IsValid(), "Invalid entity");
		PE_CORE_ASSERT(instance.HasComponent<ComponentPrefabSource>(), "Invalid prefab instance");
		ComponentPrefabSource& prefabSource = instance.GetComponent<ComponentPrefabSource>();
		PE_CORE_ASSERT(prefabSource.PrefabHandle == Handle, "Prefab mismatch");

		// Delete all children
		Scene* targetScene = instance.m_Scene;
		targetScene->DestroyChildren(instance);

		Ref<Scene> copy = Scene::Copy(m_PrefabScene);
		copy->RegenerateUUIDs(); // Regenerate IDs to avoid errors with multiple instances of the same prefab sharing the same UUIDs

		// Refresh nested prefabs
		auto view = copy->View<ComponentPrefabSource>();
		for (auto entityID : view)
		{
			Entity nestedPrefabRoot = Entity(entityID, copy.get());
			ComponentTransform& transform = nestedPrefabRoot.GetComponent<ComponentTransform>();
			if (transform.NumChildren() == 0)
			{
				// Refresh prefab
				ComponentPrefabSource& nestedPrefabSource = view.get<ComponentPrefabSource>(entityID);
				if (AssetManager::IsAssetHandleValid(nestedPrefabSource.PrefabHandle))
				{
					Ref<Prefab> nestedPrefabAsset = AssetManager::GetAsset<Prefab>(nestedPrefabSource.PrefabHandle);
					if (nestedPrefabAsset) {
						nestedPrefabAsset->RefreshInstance(nestedPrefabRoot);
					}
				}
			}
		}

		// Copy prefab scene into target scene and remap parent/child relationships from copy scene to target scene
		targetScene->Append(copy);
		std::vector<Entity> rootEntities = copy->GetRootEntities();
		for (Entity e : rootEntities)
		{
			Entity remappedEntity = targetScene->FindEntityWithUUID(e.UUID());
			ComponentTransform::SetParent(remappedEntity, instance, false);
		}
	}

	Prefab Prefab::CreateFromEntity(Entity srcEntity)
	{
		PE_PROFILE_FUNCTION();
		Prefab prefab;
		std::string prefabName = srcEntity.Tag();
		prefab.m_PrefabScene->m_Name = prefabName;
		Entity copiedEntity = srcEntity.CopyToScene(prefab.m_PrefabScene.get());
		ComponentTransform::SetParent(copiedEntity, Entity(), false);

		// Nested prefabs
		// Find any root prefab entities in the new scene graph
		// Slaughter all of the prefab children before their parents eyes
		// (leaving only the prefab roots with the prefab source component as a leaf entity)
		auto view = prefab.m_PrefabScene->View<ComponentPrefabSource>();
		for (auto entityID : view)
		{
			Entity prefabRoot = Entity(entityID, prefab.m_PrefabScene.get());
			prefab.m_PrefabScene->DestroyChildren(prefabRoot);
		}

		return prefab;
	}
}