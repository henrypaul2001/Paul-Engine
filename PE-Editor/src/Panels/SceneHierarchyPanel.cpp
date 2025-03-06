#include "SceneHierarchyPanel.h"
#include <imgui.h>
#include <imgui_internal.h>

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

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
			m_SelectedEntity = Entity();
		}

		if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems)) {
			ImGui::Text("Scene");
			ImGui::Separator();
			if (ImGui::BeginMenu("Add...")) {
				if (ImGui::MenuItem("Empty Entity")) {
					m_SelectedEntity = m_Context->CreateEntity("Empty Entity");
				}
				ImGui::Spacing();
				if (ImGui::MenuItem("Sprite")) {
					Entity sprite = m_Context->CreateEntity("Sprite");
					sprite.AddComponent<Component2DSprite>();
					m_SelectedEntity = sprite;
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties");
		if (m_SelectedEntity) {
			DrawComponents(m_SelectedEntity);

			if (ImGui::Button("Add Component...")) {
				ImGui::OpenPopup("AddComponent");
			}

			if (ImGui::BeginPopup("AddComponent")) {
				
				if (ImGui::MenuItem("Camera Component")) {
					m_SelectedEntity.AddComponent<ComponentCamera>();
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("Sprite Component")) {
					m_SelectedEntity.AddComponent<Component2DSprite>();
					ImGui::CloseCurrentPopup();
				}
				
				ImGui::EndPopup();
			}
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		const std::string& tag = entity.GetComponent<ComponentTag>().Tag;

		ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity.GetID(), flags, tag.c_str());
		if (ImGui::IsItemClicked()) {
			m_SelectedEntity = entity;
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem()) {
			ImGui::Text(tag.c_str());
			ImGui::Separator();
			if (ImGui::MenuItem("Delete")) {
				entityDeleted = true;
			}

			ImGui::EndPopup();
		}

		if (opened) {
			ImGui::Text("Hello");
			ImGui::TreePop();
		}

		if (entityDeleted) {
			if (m_SelectedEntity == entity) {
				m_SelectedEntity = Entity();
			}
			m_Context->DestroyEntity(entity);
		}
	}

	static bool DrawVec3Control(const std::string& label, glm::vec3& value, float resetValue = 0.0f, float columnWidth = 100.0f) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		bool edited = false;

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = ImVec2(lineHeight + 3.0f, lineHeight);

		ImGui::PushStyleColor(ImGuiCol_Button,			ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,	ImVec4(0.9f, 0.2f, 0.25f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,	ImVec4(1.0f, 0.05f, 0.1f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize)) {
			value.x = resetValue;
			edited = true;
		}
		ImGui::PopStyleColor(3);
		ImGui::PopFont();

		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &value.x, 0.1f, 0.0f, 0.0f, "%.2f")) { edited = true; }
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button,			ImVec4(0.15f, 0.8f, 0.1f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,	ImVec4(0.25f, 0.9f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,	ImVec4(0.1f, 1.0f, 0.05f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize)) {
			value.y = resetValue;
			edited = true;
		}
		ImGui::PopStyleColor(3);
		ImGui::PopFont();

		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &value.y, 0.1f, 0.0f, 0.0f, "%.2f")) { edited = true; }
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button,			ImVec4(0.1f, 0.15f, 0.8f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,	ImVec4(0.2f, 0.25f, 0.8f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,	ImVec4(0.05f, 0.1f, 1.0f, 1.0f));
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize)) {
			value.z = resetValue;
			edited = true;
		}
		ImGui::PopStyleColor(3);
		ImGui::PopFont();

		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &value.z, 0.1f, 0.0f, 0.0f, "%.2f")) { edited = true; }
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

		return edited;
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, bool canBeRemoved, UIFunction uiFunction) {
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		if (entity.HasComponent<T>()) {
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			bool open = (ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str()));
			if (canBeRemoved) {
				ImGui::SameLine(ImGui::GetWindowWidth() - 40.0f);
				if (ImGui::Button("...", ImVec2(25, 20))) {
					ImGui::OpenPopup("ComponentSettings");
				}
			}
			ImGui::PopStyleVar();

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings")) {

				if (ImGui::MenuItem("Remove component")) {
					removeComponent = true;
				}

				ImGui::EndPopup();
			}

			if (open) {
				T& component = entity.GetComponent<T>();
				uiFunction(component);
				ImGui::TreePop();
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (removeComponent) {
				entity.RemoveComponent<T>();
			}
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		// Tag
		DrawComponent<ComponentTag>("Tag Component", entity, false, [](ComponentTag& component) {
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), component.Tag.c_str());
			if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
				component.Tag = std::string(buffer);
			}
		});

		// Transform
		DrawComponent<ComponentTransform>("Transform Component", entity, false, [](ComponentTransform& component) {
			DrawVec3Control("Position", component.Position, 0.0f);
			glm::vec3 rotationDegrees = glm::degrees(component.Rotation);
			if (DrawVec3Control("Rotation", rotationDegrees, 0.0f)) {
				component.Rotation = glm::radians(rotationDegrees);
			}
			DrawVec3Control("Scale", component.Scale, 1.0f);
		});

		// Camera
		DrawComponent<ComponentCamera>("Camera Component", entity, true, [](ComponentCamera& component) {
			const char* projectionTypeStrings[] = { "Orthographic", "Perspective" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)component.Camera.IsPerspective()];
			if (ImGui::BeginCombo("Projection", currentProjectionTypeString)) {

				for (int i = 0; i < 2; i++) {
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected)) {
						currentProjectionTypeString = projectionTypeStrings[i];
						component.Camera.SwitchProjectionType((SceneCameraType)i);
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			bool propertyChanged = false;
			float vfov = component.Camera.GetVFOV();
			float orthoSize = component.Camera.GetOrthoSize();
			float nearClip = component.Camera.GetNearClip();
			float farClip = component.Camera.GetFarClip();
			float aspectRatio = component.Camera.GetAspectRatio();

			if (component.Camera.IsPerspective()) {
				if (ImGui::DragFloat("FOV", &vfov, 0.5f)) { propertyChanged = true; }
			}
			else {
				if (ImGui::DragFloat("Size", &orthoSize, 0.5f)) { propertyChanged = true; }
			}

			if (ImGui::DragFloat("Asepct Ratio", &aspectRatio, 0.1f)) { propertyChanged = true; }
			if (ImGui::DragFloat("Near Clip", &nearClip, 0.5f)) { propertyChanged = true; }
			if (ImGui::DragFloat("Far Clip", &farClip, 0.5f)) { propertyChanged = true; }

			ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);

			if (propertyChanged) {
				if (component.Camera.IsPerspective()) { component.Camera.SetPerspective(vfov, aspectRatio, nearClip, farClip); }
				else { component.Camera.SetOrthographic(orthoSize, aspectRatio, nearClip, farClip); }
			}
		});

		// Sprite
		DrawComponent<Component2DSprite>("Sprite Component", entity, true, [](Component2DSprite& component) {
			ImGui::ColorEdit4("Colour", &component.Colour[0]);
		});

	}
}