#include "SceneHierarchyPanel.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Scene/Components.h"
#include <PaulEngine/Debug/Instrumentor.h>

#include <filesystem>
#include <PaulEngine/Renderer/Asset/TextureAtlas2D.h>

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
			// Only draw root entities
			// DrawEntityNode will recursively draw children
			if (!entity.GetComponent<ComponentTransform>().GetParent().IsValid())
			{
				DrawEntityNode(entity);
			}
		});

		ImGui::Dummy(ImGui::GetContentRegionAvail());

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_ITEM"))
			{
				Entity entityPayload = *(Entity*)payload->Data;
				ComponentTransform::SetParent(entityPayload, Entity());
			}
			ImGui::EndDragDropTarget();
		}

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

	template<typename T>
	inline void SceneHierarchyPanel::DrawAddComponentEntry(const std::string& entryName)
	{
		if (!m_SelectedEntity.HasComponent<T>()) {
			if (ImGui::MenuItem(entryName.c_str())) {
				m_SelectedEntity.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		if (entity.IsValid())
		{
			const std::string& tag = entity.GetComponent<ComponentTag>().Tag;

			ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
			ComponentTransform& transform = entity.GetComponent<ComponentTransform>();
			size_t numChildren = transform.NumChildren();
			bool isLeaf = numChildren == 0;
			if (isLeaf)
			{
				flags |= ImGuiTreeNodeFlags_Leaf;
			}

			bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity.GetID(), flags, tag.c_str());

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_ITEM"))
				{
					Entity entityPayload = *(Entity*)payload->Data;
					if (entityPayload != entity) {
						ComponentTransform::SetParent(entityPayload, entity);
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload("ENTITY_ITEM", &entity, sizeof(Entity));
				ImGui::EndDragDropSource();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
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
				const std::unordered_set<Entity>& children = transform.GetChildren();
				for (auto& it : children)
				{
					DrawEntityNode(it);
				}
				ImGui::TreePop();
			}

			if (entityDeleted) {
				SetSelectedEntity(Entity());
				m_Context->DestroyEntity(entity);
			}
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
			DrawAddComponentEntry<ComponentCamera>("Camera Component");
			DrawAddComponentEntry<Component2DSprite>("Sprite 2D Component");
			DrawAddComponentEntry<Component2DCircle>("Circle 2D Component");
			DrawAddComponentEntry<ComponentRigidBody2D>("Rigid Body 2D Component");
			DrawAddComponentEntry<ComponentBoxCollider2D>("Box Collision 2D Component");
			DrawAddComponentEntry<ComponentCircleCollider2D>("Circle Collision Component 2D");
			DrawAddComponentEntry<ComponentTextRenderer>("Text Renderer Component");
			DrawAddComponentEntry<ComponentMeshRenderer>("Mesh Renderer Component");
			DrawAddComponentEntry<ComponentDirectionalLight>("Directional Light Component");
			DrawAddComponentEntry<ComponentPointLight>("Point Light Component");
			DrawAddComponentEntry<ComponentSpotLight>("Spot Light Component");
			ImGui::EndPopup();
		}
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();

		// Transform
		DrawComponent<ComponentTransform>("Transform Component", entity, false, [entity](ComponentTransform& component) {
			glm::vec3 position = component.LocalPosition();
			glm::vec3 rotationDegrees = glm::degrees(component.LocalRotation());
			glm::vec3 scale = component.LocalScale();
			if (DrawVec3Control("Position", position, 0.0f)) { component.SetLocalPosition(position); }
			if (DrawVec3Control("Rotation", rotationDegrees, 0.0f)) { component.SetLocalRotation(glm::radians(rotationDegrees)); }
			if (DrawVec3Control("Scale", scale, 1.0f)) { component.SetLocalScale(scale); }

			ImGui::Spacing();

			std::string label = "None";
			bool isEntityValid = false;
			Entity parentEntity = component.GetParent();
			if (parentEntity.IsValid()) {
				label = parentEntity.GetComponent<ComponentTag>().Tag;
				isEntityValid = true;
			}

			ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
			buttonLabelSize.x += 20.0f;
			float buttonLabelWidth = glm::max<float>(100.0f, buttonLabelSize.x);

			ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_ITEM"))
				{
					Entity entityPayload = *(Entity*)payload->Data;
					if (entityPayload != entity) {
						ComponentTransform::SetParent(entity, entityPayload);
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (isEntityValid) {
				ImGui::SameLine();
				ImVec2 xLabelSize = ImGui::CalcTextSize("X");
				float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
				if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) {
					ComponentTransform::SetParent(entity, Entity());
				}
			}
			ImGui::SameLine();
			ImGui::Text("Parent");
			ImGui::Text("Children: %d", component.NumChildren());
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

			ImGui::DragFloat("Gamma", &component.Camera.Gamma, 0.01f, 0.0f, 100.0f);
			ImGui::DragFloat("Exposure", &component.Camera.Exposure, 0.01f, 0.0f, 100.0f);

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

			std::string label = "None";
			bool isTextureValid = false;
			if (component.Texture != 0) {
				if (AssetManager::IsAssetHandleValid(component.Texture) && AssetManager::GetAssetType(component.Texture) == AssetType::Texture2D) {
					const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(component.Texture);
					label = metadata.FilePath.filename().string();
					isTextureValid = true;
				}
				else {
					label = "Invalid";
				}
			}

			ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
			buttonLabelSize.x += 20.0f;
			float buttonLabelWidth = glm::max<float>(100.0f, buttonLabelSize.x);

			ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					AssetHandle handle = *(AssetHandle*)payload->Data;
					if (AssetManager::GetAssetType(handle) == AssetType::Texture2D) {
						component.Texture = handle;
						component.TextureAtlas = 0;
						component.SelectedSubTextureName = "";
					}
					else if (AssetManager::GetAssetType(handle) == AssetType::TextureAtlas2D) {
						component.TextureAtlas = handle;
						Ref<TextureAtlas2D> atlas = AssetManager::GetAsset<TextureAtlas2D>(handle);
						component.Texture = atlas->GetBaseTexture();
						component.SelectedSubTextureName = "";
					}
					else {
						PE_CORE_WARN("Invalid asset type. Texture2D or TextureAtlas2D needed for sprite component");
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (isTextureValid) {
				ImGui::SameLine();
				ImVec2 xLabelSize = ImGui::CalcTextSize("X");
				float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
				if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) {
					component.Texture = 0;
					component.TextureAtlas = 0;
					component.SelectedSubTextureName = "";
				}
			}
			ImGui::SameLine();
			ImGui::Text("Texture");

			bool isAtlasValid = (AssetManager::IsAssetHandleValid(component.TextureAtlas) && AssetManager::GetAssetType(component.TextureAtlas) == AssetType::TextureAtlas2D);
			std::string selectedSubTexture = (component.TextureAtlas != 0 && component.SelectedSubTextureName == "") ? "None" : component.SelectedSubTextureName;

			if (isAtlasValid) {
				Ref<TextureAtlas2D> atlas = AssetManager::GetAsset<TextureAtlas2D>(component.TextureAtlas);
				std::vector<std::string> copiedNames = atlas->GetSubTextureNames();
				copiedNames.insert(copiedNames.begin(), { "None" });

				if (ImGui::BeginCombo("Sub Texture", selectedSubTexture.c_str())) {

					for (int i = 0; i < copiedNames.size(); i++) {
						bool isSelected = selectedSubTexture == copiedNames[i];
						if (ImGui::Selectable(copiedNames[i].c_str(), isSelected)) {
							selectedSubTexture = copiedNames[i];
							if (i == 0) {
								component.SelectedSubTextureName = "";
							}
							else {
								component.SelectedSubTextureName = selectedSubTexture;
							}
						}

						if (isSelected) {
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndCombo();
				}
			}

			if (!isAtlasValid || selectedSubTexture == "None") {
				bool edited = DrawVec2Control("Texture Scale", component.TextureScale, 1.0f);
			}

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
			const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type()];
			if (ImGui::BeginCombo("Body Type", currentBodyTypeString)) {

				for (int i = 0; i < 2; i++) {
					bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
					if (ImGui::Selectable(bodyTypeStrings[i], isSelected)) {
						currentBodyTypeString = bodyTypeStrings[i];
						component.SetType((ComponentRigidBody2D::BodyType)i);
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			bool fixedRotation = component.FixedRotation();
			if (ImGui::Checkbox("Fixed Rotation", &fixedRotation)) {
				component.SetFixedRotation(fixedRotation);
			}
		});

		// Box Collider 2D
		DrawComponent<ComponentBoxCollider2D>("Box Collider 2D", entity, true, [](ComponentBoxCollider2D& component) {
			glm::vec2 size = component.Size();
			if (DrawVec2Control("Size", size, 0.5f)) {
				component.SetSize(size);
			}

			float density = component.Density();
			float friction = component.Friction();
			float restitution = component.Restitution();

			if (ImGui::DragFloat("Density", &density, 0.1f)) { component.SetDensity(density); }
			if (ImGui::DragFloat("Friction", &friction, 0.1f, 0.0f, 1.0f)) { component.SetFriction(friction); }
			if (ImGui::DragFloat("Restitution", &restitution, 0.1f, 0.0f, 1.0f)) { component.SetRestitution(restitution); }
		});

		// Circle Collider 2D
		DrawComponent<ComponentCircleCollider2D>("Circle Collider 2D", entity, true, [](ComponentCircleCollider2D& component) {
			float radius = component.Radius();
			float density = component.Density();
			float friction = component.Friction();
			float restitution = component.Restitution();

			if (ImGui::DragFloat("Radius", &radius, 0.1f)) { component.SetRadius(radius); }

			if (ImGui::DragFloat("Density", &density, 0.1f)) { component.SetDensity(density); }
			if (ImGui::DragFloat("Friction", &friction, 0.1f, 0.0f, 1.0f)) { component.SetFriction(friction); }
			if (ImGui::DragFloat("Restitution", &restitution, 0.1f, 0.0f, 1.0f)) { component.SetRestitution(restitution); }
		});

		// Text Component
		DrawComponent<ComponentTextRenderer>("Text Renderer", entity, true, [](ComponentTextRenderer& component) {
			ImGui::InputTextMultiline("Text String", &component.TextString);

			{
				std::string label = "None";
				bool isFontValid = false;
				if (component.Font != 0) {
					if (AssetManager::IsAssetHandleValid(component.Font) && AssetManager::GetAssetType(component.Font) == AssetType::Font) {
						const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(component.Font);
						label = metadata.FilePath.filename().string();
						isFontValid = true;
					}
					else {
						label = "Invalid";
					}
				}

				ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
				buttonLabelSize.x += 20.0f;
				float buttonLabelWidth = glm::max<float>(100.0f, buttonLabelSize.x);

				ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						AssetHandle handle = *(AssetHandle*)payload->Data;
						if (AssetManager::GetAssetType(handle) == AssetType::Font) {
							component.Font = handle;
						}
						else {
							PE_CORE_WARN("Invalid asset type. Font needed for text component");
						}
					}
					ImGui::EndDragDropTarget();
				}

				if (isFontValid) {
					ImGui::SameLine();
					ImVec2 xLabelSize = ImGui::CalcTextSize("X");
					float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
					if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) {
						component.Font = Font::s_DefaultFont;
					}
				}
				ImGui::SameLine();
				ImGui::Text("Font");
			}

			ImGui::ColorEdit4("Colour", &component.Colour[0]);
			ImGui::DragFloat("Kerning", &component.Kerning, 0.025f);
			ImGui::DragFloat("Line Spacing", &component.LineSpacing, 0.025f);
		});

		// Mesh Component
		DrawComponent<ComponentMeshRenderer>("Mesh Renderer", entity, true, [](ComponentMeshRenderer& component) {

			std::string label = "None";
			bool isMaterialValid = false;
			if (component.MaterialHandle != 0) {
				if (AssetManager::IsAssetHandleValid(component.MaterialHandle) && AssetManager::GetAssetType(component.MaterialHandle) == AssetType::Material) {
					const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(component.MaterialHandle);
					label = metadata.FilePath.filename().stem().string();
					isMaterialValid = true;
				}
				else {
					label = "Invalid";
				}
			}

			ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
			buttonLabelSize.x += 20.0f;
			float buttonLabelWidth = glm::max<float>(100.0f, buttonLabelSize.x);

			ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					AssetHandle handle = *(AssetHandle*)payload->Data;
					if (AssetManager::GetAssetType(handle) == AssetType::Material) {
						component.MaterialHandle = handle;
					}
					else {
						PE_CORE_WARN("Invalid asset type. Material needed for mesh component");
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (isMaterialValid) {
				ImGui::SameLine();
				ImVec2 xLabelSize = ImGui::CalcTextSize("X");
				float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
				if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) {
					component.MaterialHandle = 0;
				}
			}
			ImGui::SameLine();
			ImGui::Text("Material");

			ImGui::Spacing();
			ImGui::Text("Depth State");

			const char* depthFuncStrings[] = {
				"Never",
				"Less", "Less or Equal",
				"Equal",
				"Greater", "Greater or Equal",
				"Not Equal",
				"Always"
			};
			const char* currentDepthFuncString = depthFuncStrings[(int)component.DepthState.Func];
			if (ImGui::BeginCombo("Func", currentDepthFuncString)) {
			
				for (int i = 0; i < 8; i++) {
					bool isSelected = currentDepthFuncString == depthFuncStrings[i];
					if (ImGui::Selectable(depthFuncStrings[i], isSelected)) {
						currentDepthFuncString = depthFuncStrings[i];
						component.DepthState.Func = (DepthFunc)i;
					}
			
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
			
				ImGui::EndCombo();
			}
			ImGui::Checkbox("Test", &component.DepthState.Test);
			ImGui::Checkbox("Write", &component.DepthState.Write);

			ImGui::Spacing();
			ImGui::Text("Culling");

			const char* faceCullingStrings[] = {
				"None",
				"Front",
				"Back",
				"Front and Back"
			};
			const char* currentFaceCullingString = faceCullingStrings[(int)component.CullState];
			if (ImGui::BeginCombo("Face Culling", currentFaceCullingString)) {

				for (int i = 0; i < 4; i++) {
					bool isSelected = currentFaceCullingString == faceCullingStrings[i];
					if (ImGui::Selectable(faceCullingStrings[i], isSelected)) {
						currentFaceCullingString = faceCullingStrings[i];
						component.CullState = (FaceCulling)i;
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
		});
	
		// Directional Light
		DrawComponent<ComponentDirectionalLight>("Directional Light", entity, true, [](ComponentDirectionalLight& component) {
			ImGui::ColorEdit3("Diffuse", &component.Diffuse[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
			ImGui::ColorEdit3("Specular", &component.Specular[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
			ImGui::ColorEdit3("Ambient", &component.Ambient[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

			ImGui::SeparatorText("Shadows");
			ImGui::Checkbox("Cast shadows", &component.CastShadows);
			if (component.CastShadows) {
				ImGui::DragFloat("Min bias", &component.ShadowMinBias, 0.001f);
				ImGui::DragFloat("Max bias", &component.ShadowMaxBias, 0.001f);
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Text("Shadow map properties");
				ImGui::DragFloat("Camera distance", &component.ShadowMapCameraDistance, 0.1f, 0.0f, 1000000.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::DragFloat("Projection size", &component.ShadowMapProjectionSize, 0.1f, 1.0f, 1000000.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::DragFloat("Near clip", &component.ShadowMapNearClip, 0.01f);
				ImGui::DragFloat("Far clip", &component.ShadowMapFarClip, 0.1f);
			}
		});

		// Point Light
		DrawComponent<ComponentPointLight>("Point Light", entity, true, [](ComponentPointLight& component) {
			ImGui::DragFloat("Radius", &component.Radius, 1.0f, 0.0f);
			ImGui::ColorEdit3("Diffuse", &component.Diffuse[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
			ImGui::ColorEdit3("Specular", &component.Specular[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
			ImGui::ColorEdit3("Ambient", &component.Ambient[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

			ImGui::SeparatorText("Shadows");
			ImGui::Checkbox("Cast shadows", &component.CastShadows);
			if (component.CastShadows) {
				ImGui::DragFloat("Min bias", &component.ShadowMinBias, 0.001f);
				ImGui::DragFloat("Max bias", &component.ShadowMaxBias, 0.001f);
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Text("Shadow map properties");
				ImGui::DragFloat("Near clip", &component.ShadowMapNearClip, 0.01f);
				ImGui::DragFloat("Far clip", &component.ShadowMapFarClip, 0.1f);
			}
		});
	
		// Spot Light
		DrawComponent<ComponentSpotLight>("Spot Light", entity, true, [](ComponentSpotLight& component) {
			ImGui::DragFloat("Range", &component.Range, 1.0f, 0.0f);
			ImGui::DragFloat("Inner Cutoff", &component.InnerCutoff);
			ImGui::DragFloat("Outer Cutoff", &component.OuterCutoff);
			ImGui::ColorEdit3("Diffuse", &component.Diffuse[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
			ImGui::ColorEdit3("Specular", &component.Specular[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
			ImGui::ColorEdit3("Ambient", &component.Ambient[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

			ImGui::SeparatorText("Shadows");
			ImGui::Checkbox("Cast shadows", &component.CastShadows);
			if (component.CastShadows) {
				ImGui::DragFloat("Min bias", &component.ShadowMinBias, 0.001f);
				ImGui::DragFloat("Max bias", &component.ShadowMaxBias, 0.001f);
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Text("Shadow map properties");
				ImGui::DragFloat("Near clip", &component.ShadowMapNearClip, 0.01f);
				ImGui::DragFloat("Far clip", &component.ShadowMapFarClip, 0.1f);
			}
		});
	}
}