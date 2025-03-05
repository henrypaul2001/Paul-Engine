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

		if (entity.HasComponent<ComponentCamera>()) {
			if (ImGui::TreeNodeEx((void*)typeid(ComponentCamera).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera Component")) {
				ComponentCamera& camera = entity.GetComponent<ComponentCamera>();

				const char* projectionTypeStrings[] = { "Orthographic", "Perspective" };
				const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.Camera.IsPerspective()];
				if (ImGui::BeginCombo("Projection", currentProjectionTypeString)) {

					for (int i = 0; i < 2; i++) {
						bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
						if (ImGui::Selectable(projectionTypeStrings[i], isSelected)) {
							currentProjectionTypeString = projectionTypeStrings[i];
							camera.Camera.SwitchProjectionType((SceneCameraType)i);
						}

						if (isSelected) {
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndCombo();
				}

				bool propertyChanged = false;
				float vfov = camera.Camera.GetVFOV();
				float orthoSize = camera.Camera.GetOrthoSize();
				float nearClip = camera.Camera.GetNearClip();
				float farClip = camera.Camera.GetFarClip();
				float aspectRatio = camera.Camera.GetAspectRatio();

				if (camera.Camera.IsPerspective()) {
					if (ImGui::DragFloat("FOV", &vfov, 0.5f)) { propertyChanged = true; }
				}
				else {
					if (ImGui::DragFloat("Size", &orthoSize, 0.5f)) { propertyChanged = true; }
				}

				if (ImGui::DragFloat("Asepct Ratio", &aspectRatio, 0.1f)) { propertyChanged = true; }
				if (ImGui::DragFloat("Near Clip", &nearClip, 0.5f)) { propertyChanged = true; }
				if (ImGui::DragFloat("Far Clip", &farClip, 0.5f)) { propertyChanged = true; }

				ImGui::Checkbox("Fixed Aspect Ratio", &camera.FixedAspectRatio);

				if (propertyChanged) {
					if (camera.Camera.IsPerspective()) { camera.Camera.SetPerspective(vfov, aspectRatio, nearClip, farClip); }
					else							   { camera.Camera.SetOrthographic(orthoSize, aspectRatio, nearClip, farClip); }
				}

				ImGui::TreePop();
			}
		}

		if (entity.HasComponent<Component2DSprite>()) {
			if (ImGui::TreeNodeEx((void*)typeid(Component2DSprite).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Sprite Renderer")) {
				Component2DSprite& spriteRenderer = entity.GetComponent<Component2DSprite>();

				ImGui::ColorEdit4("Colour", &spriteRenderer.Colour[0]);

				ImGui::TreePop();
			}
		}
	}
}