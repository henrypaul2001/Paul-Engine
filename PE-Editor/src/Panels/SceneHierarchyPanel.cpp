#include "SceneHierarchyPanel.h"
#include <imgui.h>
#include "PaulEngine/Scene/Components.h"
#include <PaulEngine/Debug/Instrumentor.h>

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
		PE_PROFILE_FUNCTION();

		ImGui::Begin("Scene Hierarchy");

		m_Context->m_Registry.view<entt::entity>().each([this](auto entityID) {
			Entity entity = Entity(entityID, m_Context.get());

			DrawEntityNode(entity);
		});
		ImGui::End();

		ImGui::Begin("Properties");
		if (m_SelectedEntity) {
			DrawComponents(m_SelectedEntity);
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		const std::string& tag = entity.GetComponent<ComponentTag>().Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity.GetID(), flags, tag.c_str());
		if (ImGui::IsItemClicked()) {
			m_SelectedEntity = entity;
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
			m_SelectedEntity = Entity();
		}

		if (opened) {
			ImGui::Text("Hello");
			ImGui::TreePop();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<ComponentTag>()) {
			if (ImGui::TreeNodeEx((void*)typeid(ComponentTag).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Tag Component")) {
				ComponentTag& tagComponent = entity.GetComponent<ComponentTag>();

				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				strcpy_s(buffer, sizeof(buffer), tagComponent.Tag.c_str());
				if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
					tagComponent.Tag = std::string(buffer);
				}

				ImGui::TreePop();
			}
		}
		
		if (entity.HasComponent<ComponentTransform>()) {
			if (ImGui::TreeNodeEx((void*)typeid(ComponentTransform).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform Component")) {
				ComponentTransform& transform = entity.GetComponent<ComponentTransform>();

				ImGui::DragFloat3("Position", &transform.Position[0], 0.1f);
				ImGui::DragFloat3("Scale", &transform.Scale[0], 0.1f);
			
				ImGui::TreePop();
			}
		}
	}
}