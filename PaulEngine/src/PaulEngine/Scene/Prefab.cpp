#include "pepch.h"
#include "Prefab.h"
#include "PaulEngine/Scene/Components.h"

namespace PaulEngine
{
	Prefab::Prefab(Entity srcEntity) : m_PrefabScene(CreateRef<Scene>())
	{
		std::string prefabName = srcEntity.Tag();
		m_PrefabScene->m_Name = prefabName;
		m_RootEntity = m_PrefabScene->CreateEntity(prefabName + "_Root");
		// Add hidden prefab component

		Entity copiedEntity = srcEntity.CopyToScene(m_PrefabScene);
		ComponentTransform::SetParent(copiedEntity, m_RootEntity);
	}

	void Prefab::Instantiate(Ref<Scene> targetScene)
	{
		if (targetScene) {
			targetScene->Append(m_PrefabScene);
		}
	}
}