#include "pepch.h"
#include "CreateTextureArrayWindow.h"

#include <PaulEngine/Project/Project.h>
#include <PaulEngine/Utils/PlatformUtils.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm/glm.hpp>
#include <imgui_internal.h>

namespace PaulEngine
{
	CreateTextureArrayWindow::CreateTextureArrayWindow() : m_ViewportSize(128.0f, 128.0f), m_CurrentConstraint({ 0, 0, 0 })
	{
		PE_PROFILE_FUNCTION();
		FramebufferSpecification spec;
		spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		spec.Width = 1280;
		spec.Height = 720;
		m_Framebuffer = Framebuffer::Create(spec);

		m_Camera = SceneCamera(SCENE_CAMERA_PERSPECTIVE);

		m_QuadTransform = glm::mat4(1.0f);
		m_QuadTransform = glm::translate(m_QuadTransform, glm::vec3(0.0f, 0.0f, -0.5f));
	}

	void CreateTextureArrayWindow::OnImGuiRender()
	{
		PE_PROFILE_FUNCTION();
		if (m_ShowWindow)
		{
			ImGui::Begin("Create texture array asset");
			ImGui::Text("Dimensions: %d x %d", m_CurrentConstraint.Width, m_CurrentConstraint.Height);
			ImGui::Text("Channels: %d", m_CurrentConstraint.Channels);
			if (ImGui::Button("Add new layer..")) {
				std::filesystem::path selectedFile = FileDialogs::OpenFile("PNG Files (*.png)\0*.png\0JPG Files (*.jpg)\0*.jpg\0TGA Files (*.TGA)\0*.tga\0BMP Files (*.bmp)\0*.bmp\0PSD Files (*.psd)\0*.psd\0GIF Files (*.gif)\0*.gif\0HDR Files (*.hdr)\0*.hdr\0PIC Files (*.pic)\0*.pic\0");
				TextureImporter::ImageFileReadResult result;
				Buffer imageBuffer = TextureImporter::ReadImageFile(selectedFile, result);
				
				bool valid = true;
				if (m_Buffers.size() > 0) {
					if (result != m_CurrentConstraint) {
						PE_CORE_ERROR("Selected file does not match current array constraints:");
						PE_CORE_ERROR("    - Selected file result:");
						PE_CORE_ERROR("        Width: {0}", result.Width);
						PE_CORE_ERROR("        Height: {0}", result.Height);
						PE_CORE_ERROR("        Channels: {0}", result.Channels);
						PE_CORE_ERROR("    - Current constraints:");
						PE_CORE_ERROR("        Width: {0}", m_CurrentConstraint.Width);
						PE_CORE_ERROR("        Height: {0}", m_CurrentConstraint.Height);
						PE_CORE_ERROR("        Channels: {0}", m_CurrentConstraint.Channels);
					
						imageBuffer.Release();
						valid = false;
					}
				}

				if (valid) {
					m_Buffers.push_back(imageBuffer);
					m_ImageNames.push_back(selectedFile.stem().string());
					m_CurrentConstraint = result;
				}
			}

			// Left
			static int selected = -1;
			static int textureDeleted = -1;
			{
				ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
				for (int i = 0; i < m_Buffers.size(); i++) {
					ImGui::PushID(i);
					if (ImGui::Selectable(m_ImageNames[i].c_str(), selected == i, ImGuiSelectableFlags_SelectOnNav)) {
						selected = i;
					}
					if (ImGui::BeginPopupContextItem()) {
						if (ImGui::MenuItem("Delete")) {
							textureDeleted = i;
							if (textureDeleted == selected) {
								selected = -1;
							}
						}
						ImGui::EndPopup();
					}
					ImGui::PopID();
				}
				ImGui::EndChild();
			}
			ImGui::SameLine();

			// Right
			{
				ImGui::BeginGroup();
				ImGui::BeginChild("preview", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
				if (selected != -1) {
					ImGui::Text(m_ImageNames[selected].c_str());
					ImGui::SameLine();
					ImGui::Text("   (layer: %d)", selected);
				}
				ImGui::EndChild();

				if (ImGui::Button("Cancel")) {
					m_ShowWindow = false;
					for (Buffer b : m_Buffers) {
						b.Release();
					}
					m_Buffers.clear();
					m_ImageNames.clear();
					m_CurrentConstraint = { 0,0,0 };
					selected = -1;
				}
				ImGui::SameLine();
				if (ImGui::Button("Save As..")) {
					std::filesystem::path path = FileDialogs::SaveFile("Binary Texture Array (.bta)\0*.bta\0");
					if (!path.empty()) {
						std::filesystem::path absoluteProjectPath = std::filesystem::absolute(Project::GetProjectDirectory());
						std::filesystem::path relativeSavePath = std::filesystem::path(path).lexically_relative(absoluteProjectPath.parent_path());

						TextureSpecification spec;
						spec.Width = m_CurrentConstraint.Width;
						spec.Height = m_CurrentConstraint.Height;

						switch (m_CurrentConstraint.Channels)
						{
						case 4:
							spec.Format = ImageFormat::RGBA8;
							break;
						case 3:
							spec.Format = ImageFormat::RGB8;
							break;
						case 2:
							spec.Format = ImageFormat::RG8;
							break;
						case 1:
							spec.Format = ImageFormat::R8;
							break;
						}

						TextureImporter::SaveBTAFile(relativeSavePath, m_Buffers, spec);

						m_ShowWindow = false;
						for (Buffer b : m_Buffers) {
							b.Release();
						}
						m_Buffers.clear();
						m_ImageNames.clear();
						m_CurrentConstraint = { 0,0,0 };
						selected = -1;
					}
				}
				ImGui::EndGroup();
			}

			ImGui::End();
		
			if (textureDeleted != -1) {
				m_Buffers[textureDeleted].Release();
				m_Buffers.erase(m_Buffers.begin() + textureDeleted);
				m_ImageNames.erase(m_ImageNames.begin() + textureDeleted);

				if (m_Buffers.size() < 1) {
					m_CurrentConstraint = { 0, 0, 0 };
				}

				textureDeleted = -1;
			}
		}
	}
}