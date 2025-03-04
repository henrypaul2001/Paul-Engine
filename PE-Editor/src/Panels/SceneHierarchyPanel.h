#pragma once
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Scene/Scene.h"
#include "PaulEngine/Scene/Entity.h"

namespace PaulEngine
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() {}
		SceneHierarchyPanel(const Ref<Scene>& context);

		void SetContext(const Ref<Scene>& context);

		void OnImGuiRender();
	
	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

		Ref<Scene> m_Context;
		Entity m_SelectedEntity;
	};
}