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
	CreateAtlasWindow::CreateAtlasWindow(AssetHandle baseTexture) : m_BaseTexture(baseTexture), m_ViewportSize(128.0f, 128.0f) {}

	void CreateAtlasWindow::Init()
	{
		FramebufferSpecification spec;
		spec.Width = 1280;
		spec.Height = 720;
		spec.Samples = 1;

		TextureSpecification texSpec;
		texSpec.Width = 1280;
		texSpec.Height = 720;
		texSpec.GenerateMips = false;
		texSpec.Format = ImageFormat::RGBA8;
		texSpec.MinFilter = ImageMinFilter::NEAREST;
		texSpec.MagFilter = ImageMagFilter::NEAREST;
		texSpec.Wrap_S = ImageWrap::CLAMP_TO_BORDER;
		texSpec.Wrap_T = ImageWrap::CLAMP_TO_BORDER;
		texSpec.Wrap_R = ImageWrap::CLAMP_TO_BORDER;
		texSpec.Border = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		Ref<FramebufferTexture2DAttachment> colour0Attach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, texSpec, true);

		texSpec.Format = ImageFormat::RED_INTEGER;
		Ref<FramebufferTexture2DAttachment> entityIDAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour1, texSpec, true);

		texSpec.Format = ImageFormat::Depth24Stencil8;
		Ref<FramebufferTexture2DAttachment> depthAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::DepthStencil, texSpec, true);

		m_Framebuffer = Framebuffer::Create(spec, { colour0Attach, entityIDAttach }, depthAttach);

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

			// TODO: Add support for setting the space between tiles in pixels. For example, 16x16 tile size with 1x1 pixels between tiles

			// Left
			static int selected = -1;
			static int subTextureDeleted = -1;
			static glm::vec2 cellSize = glm::vec2(128.0f);
			static glm::vec2 lastSpriteSize = glm::vec2(1.0f);
			glm::ivec2 cellCount = ((glm::ivec2)textureSize / (glm::ivec2)cellSize) - glm::ivec2(1, 1);
			{
				ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);

				ImGui::Text("Sheet dimensions: %d x %d", (int)textureSize.x, (int)textureSize.y);
				ImGui::Text("Cell count: %d x %d", cellCount.x + 1, cellCount.y + 1);
				ImGui::DragFloat2("Cell size", &cellSize[0], 1.0f, 1.0f, 1000000.0f, "%.1f");
				cellSize = glm::min(cellSize, textureSize);
				cellSize = glm::max(glm::vec2(1.0f), cellSize);
				if (ImGui::Button("Add new sub-texture")) {
					ImGui::OpenPopup("Add new sub-texture");
				}
				ImGui::SameLine();
				if (ImGui::Button("Auto create sub-textures")) {
					ImGui::OpenPopup("Auto create sub-textures");
				}
				if (ImGui::BeginPopupModal("Add new sub-texture")) {
					ImGui::Text("Create sub-texture");

					static std::string name = "";
					ImGui::InputText("Name", &name);

					static glm::vec2 cellCoords = glm::vec2(0.0f);
					ImGui::DragFloat2("Cell coords", &cellCoords[0], 1.0f, 0.0f, 0.0f, "%.1f");
					cellCoords = glm::min(cellCoords, (glm::vec2)cellCount);
					cellCoords = glm::max(glm::vec2(0.0f), cellCoords);

					static glm::vec2 spriteSize = lastSpriteSize;
					ImGui::DragFloat2("Sprite size", &spriteSize[0], 1.0f, 0.0f, 0.0f, "%.1f");
					spriteSize = glm::min(spriteSize, textureSize);
					spriteSize = glm::max(glm::vec2(0.0f), spriteSize);

					if (ImGui::Button("Cancel")) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Confirm")) {
						// Validate
						const bool uniqueName = m_NameToInputIDMap.find(name) == m_NameToInputIDMap.end();
						const bool nameIsntEmpty = (name != "");
						if (uniqueName && nameIsntEmpty) {
							int index = m_SubTextureInputList.size();
							m_SubTextureInputList.push_back({ cellCoords, spriteSize });
							m_SubTextureNames.push_back(name);
							m_NameToInputIDMap[name] = index;
							ImGui::CloseCurrentPopup();
						}
						else {
							PE_CORE_ERROR("Error creating subtexture: uniqueName = '{0}', nameIsntEmpty = '{1}'", uniqueName, nameIsntEmpty);
						}
					}
					ImGui::EndPopup();
				}
				if (ImGui::BeginPopupModal("Auto create sub-textures")) {
					
					static std::string baseName = "SubTexture";
					ImGui::InputText("Base name", &baseName);

					static glm::vec2 startCoords = glm::vec2(0.0f);
					ImGui::DragFloat2("Starting cell coords", &startCoords[0], 1.0f, 0.0f, 0.0f, "%.1f");
					startCoords = glm::min(startCoords, (glm::vec2)cellCount);
					startCoords = glm::max(glm::vec2(0.0f), startCoords);

					static glm::ivec2 cellsToCreate = glm::vec2(1.0f);
					ImGui::DragInt2("Sub-textures to create", &cellsToCreate[0], 1.0f, 0);
					cellsToCreate = glm::min(cellsToCreate, (cellCount - (glm::ivec2)startCoords));
					
					if (ImGui::Button("Cancel")) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Confirm")) {
						for (int x = 0; x < cellsToCreate.x; x++) {
							for (int y = 0; y < cellsToCreate.y; y++) {
								std::string indexedName = baseName + std::to_string(x) + "X" + std::to_string(y);
								std::string name = indexedName;
								int count = 1;
								while (m_NameToInputIDMap.find(name) != m_NameToInputIDMap.end()) {
									name = indexedName + "(" + std::to_string(count) + ")";
									count++;
								}

								glm::vec2 cellCoords = startCoords + glm::vec2(x, y);
								glm::vec2 spriteSize = glm::vec2(1.0f);

								int index = m_SubTextureInputList.size();
								m_SubTextureInputList.push_back({ cellCoords, spriteSize });
								m_SubTextureNames.push_back(name);
								m_NameToInputIDMap[name] = index;
							}
						}
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
				for (int i = 0; i < m_SubTextureInputList.size(); i++) {
					if (ImGui::Selectable(m_SubTextureNames[i].c_str(), selected == i, ImGuiSelectableFlags_SelectOnNav)) {
						selected = i;
					}
					if (ImGui::BeginPopupContextItem()) {
						if (ImGui::MenuItem("Delete")) {
							subTextureDeleted = i;
							if (subTextureDeleted == selected) {
								selected = -1;
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
				if (selected != -1) {
					ImGui::Text(m_SubTextureNames[selected].c_str());
					ImGui::SameLine();
					if (ImGui::Button("Rename")) {
						ImGui::OpenPopup("Rename sub-texture");
					}
					if (ImGui::BeginPopupModal("Rename sub-texture")) {
						ImGui::Text("Rename sub-texture");

						static std::string selectedName = m_SubTextureNames[selected];
						ImGui::InputText("Name", &selectedName);

						if (ImGui::Button("Cancel")) {
							ImGui::CloseCurrentPopup();
						}
						ImGui::SameLine();
						if (ImGui::Button("Confirm")) {
							// Validate
							const bool uniqueName = m_NameToInputIDMap.find(selectedName) == m_NameToInputIDMap.end();
							const bool nameIsntEmpty = (selectedName != "");
							if (uniqueName && nameIsntEmpty) {
								const std::string& previousName = m_SubTextureNames[selected];
								int index = m_NameToInputIDMap[previousName];

								m_NameToInputIDMap.erase(previousName);
								m_NameToInputIDMap[selectedName] = index;
								m_SubTextureNames[index] = selectedName;

								ImGui::CloseCurrentPopup();
							}
							else {
								PE_CORE_ERROR("Error renaming subtexture: uniqueName = '{0}', nameIsntEmpty = '{1}'", uniqueName, nameIsntEmpty);
							}
						}
						ImGui::EndPopup();
					}

					SubTextureInput* subTextureInput = &m_SubTextureInputList[selected];

					ImGui::DragFloat2("Cell coords", &subTextureInput->cellCoords[0], 1.0f, 0.0f, 0.0f, "%.1f");
					subTextureInput->cellCoords = glm::min(subTextureInput->cellCoords, (glm::vec2)cellCount);
					subTextureInput->cellCoords = glm::max(glm::vec2(0.0f), subTextureInput->cellCoords);

					ImGui::DragFloat2("Sprite size", &subTextureInput->spriteSize[0], 1.0f, 0.0f, 0.0f, "%.1f");
					subTextureInput->spriteSize = glm::min(subTextureInput->spriteSize, textureSize);
					subTextureInput->spriteSize = glm::max(glm::vec2(0.0f), subTextureInput->spriteSize);

					ImGui::Separator();

					// Render preview
					{
						glm::vec2 spriteSize = glm::vec2(1.0f);
						if (selected != -1) {
							spriteSize = m_SubTextureInputList[selected].spriteSize;
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
						if (selected != -1) {
							const SubTextureInput& input = m_SubTextureInputList[selected];
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
						FramebufferTexture2DAttachment* texAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(m_Framebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0).get());
						uint32_t textureID = texAttachment->GetTexture()->GetRendererID();
						ImGui::Image(textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
					}
				}
				ImGui::EndChild();

				if (ImGui::Button("Cancel")) {
					m_ShowWindow = false;
					m_SubTextureInputList.clear();
					m_NameToInputIDMap.clear();
					m_SubTextureNames.clear();
					m_BaseTexture = 0;
					selected = -1;
					lastSpriteSize = glm::vec2(128.0f);
				}
				ImGui::SameLine();
				if (ImGui::Button("Save As...")) {
					// Save As...
					std::string path = FileDialogs::SaveFile("Paul Engine Texture Atlas (*.patlas)\0*.patlas\0");
					if (!path.empty()) {
						TextureAtlas2D textureAtlas = TextureAtlas2D(m_BaseTexture);
						for (int i = 0; i < m_SubTextureInputList.size(); i++) {
							textureAtlas.AddSubTexture(m_SubTextureNames[i], SubTexture2D::CreateFromCoords(m_BaseTexture, m_SubTextureInputList[i].cellCoords, cellSize, m_SubTextureInputList[i].spriteSize));
						}

						std::filesystem::path absoluteProjectPath = std::filesystem::absolute(Project::GetProjectDirectory());
						std::filesystem::path relativeSavePath = std::filesystem::path(path).lexically_relative(absoluteProjectPath.parent_path());

						TextureImporter::SaveTextureAtlas2D(textureAtlas, relativeSavePath);
						AssetHandle handle = Project::GetActive()->GetEditorAssetManager()->ImportAssetFromFile(relativeSavePath.lexically_relative(Project::GetAssetDirectory()), false);

						m_ShowWindow = false;
						m_SubTextureInputList.clear();
						m_NameToInputIDMap.clear();
						m_SubTextureNames.clear();
						m_BaseTexture = 0;
						selected = -1;
						lastSpriteSize = glm::vec2(1.0f);
					}
				}
				ImGui::EndGroup();
			}

			if (!isTextureValid) {
				ImGui::EndDisabled();
			}

			ImGui::End();

			if (subTextureDeleted != -1) {
				m_NameToInputIDMap.erase(m_SubTextureNames[subTextureDeleted]);

				m_SubTextureNames.erase(m_SubTextureNames.begin() + subTextureDeleted);
				m_SubTextureInputList.erase(m_SubTextureInputList.begin() + subTextureDeleted);

				for (int i = subTextureDeleted; i < m_SubTextureNames.size(); i++) {
					m_NameToInputIDMap[m_SubTextureNames[i]] -= 1;
				}
				subTextureDeleted = -1;
			}
		}
	}
}