#include "SceneHierarchyPanel.h"
#include <imgui.h>
#include <imgui_internal.h>

#include "PaulEngine/Scene/Components.h"
#include <PaulEngine/Debug/Instrumentor.h>

#include <filesystem>

namespace PaulEngine
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		m_SelectedEntity = Entity();
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

		ImGui::SetNextWindowSizeConstraints(ImVec2(380, 0), ImVec2(FLT_MAX, FLT_MAX), 0, nullptr);
		ImGui::Begin("Properties");
		if (m_SelectedEntity) {
			DrawComponents(m_SelectedEntity);
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
	{
		m_SelectedEntity = entity;
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
			if (ImGui::MenuItem("Duplicate", "LCtrl+D")) {
				m_Context->DuplicateEntity(m_SelectedEntity);
			}
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
			SetSelectedEntity(Entity());
			m_Context->DestroyEntity(entity);
		}
	}

	static bool DrawVec2Control(const std::string& label, glm::vec2& value, float resetValue = 0.0f, float columnWidth = 100.0f) {
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

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.25f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.05f, 0.1f, 1.0f));
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

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.8f, 0.1f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.9f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 1.0f, 0.05f, 1.0f));
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

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

		return edited;
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
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>()) {
			
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();

			bool open = (ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str()));
			ImGui::PopStyleVar();

			if (canBeRemoved) {
				ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
				if (ImGui::Button("---", ImVec2(lineHeight, lineHeight))) {
					ImGui::OpenPopup("ComponentSettings");
				}
			}

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

			if (removeComponent) {
				entity.RemoveComponent<T>();
			}
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
		// Tag
		if (entity.HasComponent<ComponentTag>()) {
			ComponentTag& tagComponent = entity.GetComponent<ComponentTag>();
		
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tagComponent.Tag.c_str());
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
				tagComponent.Tag = std::string(buffer);
			}
		}
		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		
		if (ImGui::Button("Add Component")) {
			ImGui::OpenPopup("AddComponent");
		}
		
		if (ImGui::BeginPopup("AddComponent")) {
			if (!entity.HasComponent<ComponentCamera>()) {
				if (ImGui::MenuItem("Camera Component")) {
					m_SelectedEntity.AddComponent<ComponentCamera>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<Component2DSprite>()) {
				if (ImGui::MenuItem("Sprite 2D Component")) {
					m_SelectedEntity.AddComponent<Component2DSprite>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<Component2DCircle>()) {
				if (ImGui::MenuItem("Circle 2D Component")) {
					m_SelectedEntity.AddComponent<Component2DCircle>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<ComponentRigidBody2D>()) {
				if (ImGui::MenuItem("Rigid Body 2D Component")) {
					m_SelectedEntity.AddComponent<ComponentRigidBody2D>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<ComponentBoxCollider2D>()) {
				if (ImGui::MenuItem("Box Collision 2D Component")) {
					m_SelectedEntity.AddComponent<ComponentBoxCollider2D>();
					ImGui::CloseCurrentPopup();
				}
			}
			if (!entity.HasComponent<ComponentCircleCollider2D>()) {
				if (ImGui::MenuItem("Circle Collision 2D Component")) {
					m_SelectedEntity.AddComponent<ComponentCircleCollider2D>();
					ImGui::CloseCurrentPopup();
				}
			}
		
			ImGui::EndPopup();
		}
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();

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
		DrawComponent<Component2DSprite>("Sprite Renderer Component", entity, true, [](Component2DSprite& component) {
			ImGui::ColorEdit4("Colour", &component.Colour[0]);

			ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texturePath = std::filesystem::path(path);
					const std::string& extension = texturePath.extension().string();
					if (extension == ".png" || extension == ".jpg" || extension == ".JPG") {
						component.Texture = Texture2D::Create(texturePath.string());
					}
				}
				ImGui::EndDragDropTarget();
			}
			
			bool edited = DrawVec2Control("Texture Scale", component.TextureScale, 1.0f);
		});

		// Circle
		DrawComponent<Component2DCircle>("Circle Renderer Component", entity, true, [](Component2DCircle& component) {
			ImGui::ColorEdit4("Colour", &component.Colour[0]);
			ImGui::DragFloat("Thickness", &component.Thickness, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Fade", &component.Fade, 0.01f, 0.0f, 1.0f);
		});

		// Rigid Body 2D
		DrawComponent<ComponentRigidBody2D>("Rigid Body 2D", entity, true, [](ComponentRigidBody2D& component) {
			const char* bodyTypeStrings[] = { " Static", "Dynamic", "Kinematic" };
			const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
			if (ImGui::BeginCombo("Body Type", currentBodyTypeString)) {

				for (int i = 0; i < 2; i++) {
					bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
					if (ImGui::Selectable(bodyTypeStrings[i], isSelected)) {
						currentBodyTypeString = bodyTypeStrings[i];
						component.Type = (ComponentRigidBody2D::BodyType)i;
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
		});

		// Box Collider 2D
		DrawComponent<ComponentBoxCollider2D>("Box Collider 2D", entity, true, [](ComponentBoxCollider2D& component) {
			DrawVec2Control("Offset", component.Offset, 0.0f);
			DrawVec2Control("Size", component.Size, 0.5f);

			ImGui::DragFloat("Density", &component.Density, 0.1f);
			ImGui::DragFloat("Friction", &component.Friction, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.1f, 0.0f, 1.0f);
		});

		// Circle Collider 2D
		DrawComponent<ComponentCircleCollider2D>("Circle Collider 2D", entity, true, [](ComponentCircleCollider2D& component) {
			DrawVec2Control("Offset", component.Offset, 0.0f);
			ImGui::DragFloat("Radius", &component.Radius, 0.1f);

			ImGui::DragFloat("Density", &component.Density, 0.1f);
			ImGui::DragFloat("Friction", &component.Friction, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.1f, 0.0f, 1.0f);
		});
	}
}