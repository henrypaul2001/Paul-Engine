#include "pepch.h"
#include "CreateAtlasWindow.h"

#include "PaulEngine/Renderer/Renderer2D.h"
#include "PaulEngine/Renderer/RenderCommand.h"

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Asset/TextureImporter.h"

#include "PaulEngine/Renderer/TextureAtlas2D.h"
#include "PaulEngine/Utils/PlatformUtils.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/glm.hpp>
#include <imgui_internal.h>

namespace PaulEngine
{
	CreateAtlasWindow::CreateAtlasWindow(AssetHandle baseTexture) : m_BaseTexture(baseTexture), m_ViewportSize(128.0f, 128.0f)
	{
		Init();
	}

	void CreateAtlasWindow::Init()
	{
		FramebufferSpecification spec;
		spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		spec.Width = 1280;
		spec.Height = 720;
		m_Framebuffer = Framebuffer::Create(spec);

		m_Camera = SceneCamera(SCENE_CAMERA_PERSPECTIVE);
	}

	void CreateAtlasWindow::SetContext(AssetHandle baseTexture)
	{
		m_BaseTexture = baseTexture;
	}

	void CreateAtlasWindow::OnImGuiRender()
	{
		PE_PROFILE_FUNCTION();
		if (m_ShowWindow) {
			ImGui::Begin("Create texture atlas asset");

			std::string label = "None";
			bool isTextureValid = false;
			if (m_BaseTexture != 0) {
				if (AssetManager::IsAssetHandleValid(m_BaseTexture) && AssetManager::GetAssetType(m_BaseTexture) == AssetType::Texture2D) {
					const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(m_BaseTexture);
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
			buttonLabelWidth = (ImGui::GetContentRegionAvail().x * 0.9f) - ImGui::CalcTextSize("Base texture").x;

			ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					AssetHandle handle = *(AssetHandle*)payload->Data;
					if (AssetManager::GetAssetType(handle) == AssetType::Texture2D) {
						m_BaseTexture = handle;
					}
					else {
						PE_CORE_WARN("Invalid asset type. Texture2D needed for atlas creation");
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();
			ImGui::Text("Base texture");

			const Ref<Texture2D> baseTextureAsset = AssetManager::GetAsset<Texture2D>(m_BaseTexture);
			glm::vec2 textureSize = glm::vec2(0.0f);

			if (!isTextureValid) {
				ImGui::BeginDisabled();
			}
			else {
				textureSize = { (float)baseTextureAsset->GetWidth(), (float)baseTextureAsset->GetHeight() };
			}

			// Left
			static std::string selected = "";
			static std::string subTextureDeleted = "";
			static glm::vec2 cellSize = glm::vec2(128.0f);
			static glm::vec2 lastSpriteSize = glm::vec2(1.0f);
			{
				ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);

				ImGui::InputFloat2("Cell size", &cellSize[0]);
				if (ImGui::Button("Add new sub-texture")) {
					ImGui::OpenPopup("Add new sub-texture");
				}
				if (ImGui::BeginPopupModal("Add new sub-texture")) {
					ImGui::Text("Create sub-texture");

					static std::string name = "";
					ImGui::InputText("Name", &name);

					static glm::vec2 cellCoords = glm::vec2(0.0f);
					ImGui::InputFloat2("Cell coords", &cellCoords[0]);

					static glm::vec2 spriteSize = lastSpriteSize;
					ImGui::InputFloat2("Sprite size", &spriteSize[0]);

					if (ImGui::Button("Cancel")) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Confirm")) {
						// Validate
						const bool uniqueName = m_SubTextureInputMap.find(name) == m_SubTextureInputMap.end();
						const bool nameIsntEmpty = (name != "");
						if (uniqueName && nameIsntEmpty) {
							m_SubTextureInputMap[name] = { cellCoords, spriteSize };
							ImGui::CloseCurrentPopup();
						}
						else {
							PE_CORE_ERROR("Error creating subtexture: uniqueName = '{0}', nameIsntEmpty = '{1}'", uniqueName, nameIsntEmpty);
						}
					}
					ImGui::EndPopup();
				}

				for (auto& it : m_SubTextureInputMap) {
					if (ImGui::Selectable(it.first.c_str(), selected == it.first, ImGuiSelectableFlags_SelectOnNav)) {
						selected = it.first;
					}
					if (ImGui::BeginPopupContextItem()) {
						if (ImGui::MenuItem("Delete")) {
							subTextureDeleted = it.first;
							if (subTextureDeleted == selected) {
								selected = "";
							}
						}
						ImGui::EndPopup();
					}
				}
				ImGui::EndChild();
			}
			ImGui::SameLine();

			// Right
			{
				ImGui::BeginGroup();
				ImGui::BeginChild("preview", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
				if (selected != "") {
					ImGui::Text(selected.c_str());
					SubTextureInput* subTextureInput = &m_SubTextureInputMap[selected];
					ImGui::InputFloat2("Cell coords", &subTextureInput->cellCoords[0]);
					ImGui::InputFloat2("Sprite size", &subTextureInput->spriteSize[0]);
					ImGui::Separator();

					// Render preview
					{
						glm::vec2 spriteSize = glm::vec2(1.0f);
						if (selected != "") {
							spriteSize = m_SubTextureInputMap[selected].spriteSize;
						}
						glm::vec2 imageSize = cellSize * spriteSize;

						ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
						m_ViewportSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);

						// Resize
						const FramebufferSpecification& spec = m_Framebuffer->GetSpecification();
						if ((uint32_t)imageSize.x != spec.Width || (uint32_t)imageSize.y != spec.Height) {
							m_Framebuffer->Resize((uint32_t)imageSize.x, (uint32_t)imageSize.y);
							m_Camera.SetViewportSize(imageSize.x, imageSize.y);
						}

						Renderer2D::ResetStats();
						m_Framebuffer->Bind();
						RenderCommand::SetViewport({ 0.0f, 0.0f }, glm::ivec2((glm::ivec2)imageSize));
						RenderCommand::SetClearColour(glm::vec4(1.0f, 0.1f, 0.1f, 1.0f));
						RenderCommand::Clear();

						Renderer2D::BeginScene(m_Camera, glm::mat4(1.0f));
						if (selected != "") {
							const SubTextureInput& input = m_SubTextureInputMap[selected];
							Ref<SubTexture2D> selectedSubTexture = CreateRef<SubTexture2D>(SubTexture2D::CreateFromCoords(m_BaseTexture, input.cellCoords, cellSize, input.spriteSize));

							glm::vec2 quadSize = glm::vec2(spriteSize.x / spriteSize.y, 1.0f);
							m_QuadTransform = glm::mat4(1.0f);
							m_QuadTransform = glm::translate(m_QuadTransform, glm::vec3(0.0f, 0.0f, -0.5f));
							m_QuadTransform = glm::scale(m_QuadTransform, glm::vec3(quadSize, 1.0f));
							Renderer2D::DrawQuad(m_QuadTransform, selectedSubTexture, glm::vec4(1.0f), -1);
						}
						Renderer2D::EndScene();

						m_Framebuffer->Unbind();

						// Display viewport
						uint32_t textureID = m_Framebuffer->GetColourAttachmentID();
						ImGui::Image(textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
					}
				}
				ImGui::EndChild();

				if (ImGui::Button("Cancel")) {
					m_ShowWindow = false;
					m_SubTextureInputMap.clear();
					m_BaseTexture = 0;
					selected = "";
					lastSpriteSize = glm::vec2(128.0f);
				}
				ImGui::SameLine();
				if (ImGui::Button("Confirm")) {
					// Save As...
					std::string path = FileDialogs::SaveFile("Paul Engine Texture Atlas (*.patlas)\0*.patlas\0");
					if (!path.empty()) {
						TextureAtlas2D textureAtlas = TextureAtlas2D(m_BaseTexture);
						for (auto& it : m_SubTextureInputMap) {
							textureAtlas.AddSubTexture(it.first, SubTexture2D::CreateFromCoords(m_BaseTexture, it.second.cellCoords, cellSize, it.second.spriteSize));
						}

						std::filesystem::path absoluteProjectPath = std::filesystem::absolute(Project::GetActive()->GetProjectDirectory());
						std::filesystem::path relativeSavePath = std::filesystem::path(path).lexically_relative(absoluteProjectPath.parent_path());

						TextureImporter::SaveTextureAtlas2D(textureAtlas, relativeSavePath);
						AssetHandle handle = Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativeSavePath.lexically_relative(Project::GetActive()->GetAssetDirectory()), false);

						m_ShowWindow = false;
						m_SubTextureInputMap.clear();
						m_BaseTexture = 0;
						selected = "";
						lastSpriteSize = glm::vec2(1.0f);
					}
				}
				ImGui::EndGroup();
			}

			if (!isTextureValid) {
				ImGui::EndDisabled();
			}

			ImGui::End();

			if (subTextureDeleted != "") {
				m_SubTextureInputMap.erase(subTextureDeleted);
				subTextureDeleted = "";
			}
		}
	}
}