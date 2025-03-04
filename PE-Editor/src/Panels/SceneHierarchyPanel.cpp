#include "SceneHierarchyPanel.h"
#include <imgui.h>
#include "PaulEngine/Scene/Components.h"

namespace PaulEngine
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		m_Context->m_Registry.view<entt::entity>().each([this](auto entityID) {
			Entity entity = Entity(entityID, m_Context.get());
			const std::string& tag = entity.GetComponent<ComponentTag>().Tag;
			ImGui::Text("%s", tag.c_str());
		});

		ImGui::End();
	}
}