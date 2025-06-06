#pragma once
#include "PaulEngine/Asset/Asset.h"
#include "PaulEngine/Scene/Entity.h"

namespace PaulEngine
{
	// A prefab is basically just a scene.
	// It keeps its own scene instance with whatever complexity entity relationship is required
	// When it is instantiated, all of the entities from the prefab scene get copied to the target scene
	// This also makes it more suitable for editing in the editor, where scene assets are used as the context
	class Prefab : public Asset
	{
	public:
		Prefab(Entity srcEntity);
		void Instantiate(Ref<Scene> targetScene);

		virtual AssetType GetType() const override { return AssetType::Prefab; }

		const Ref<Scene> GetPrefabScene() const { return m_PrefabScene; }

	private:
		friend class EditorLayer;
		Ref<Scene> m_PrefabScene;
		Entity m_RootEntity;
	};

	// TODO: Live prefab updates
	/*
	Every entity created from a prefab should be marked as a prefab entity

	When a prefab is instantiated, the root entity of the prefab will be given a hidden prefab
	component which holds an asset handle (linking to the prefab asset)

	When the prefab asset is changed, do the following:
		- Get map of every prefab asset handle to every active prefab instance
			- PrefabHandle1 = { Entity1, Entity3, Entity4 }
			- PrefabHandle2 = { Entity7 }
			(This could be updated every time a prefab is instantiated and cached in the scene class)
		Next:
		- Example: PrefabHandle1
		- Get prefab asset from handle
		- Delete entities from scene originating from PrefabHandle1 (Entity1, Entity3, Entity4 -- also deletes children included in the prefab)
		- Re-instantiate prefab N times, where N = the original number of prefab instances
	*/
}