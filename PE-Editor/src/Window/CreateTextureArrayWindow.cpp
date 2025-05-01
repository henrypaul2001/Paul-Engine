#include "pepch.h"
#include "CreateTextureArrayWindow.h"

#include <PaulEngine/Renderer/Renderer2D.h>
#include <PaulEngine/Renderer/RenderCommand.h>

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
		Ref<FramebufferTexture2DAttachment> colour0Attach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour0, texSpec);

		texSpec.Format = ImageFormat::RED_INTEGER;
		Ref<FramebufferTexture2DAttachment> entityIDAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::Colour1, texSpec);

		texSpec.Format = ImageFormat::Depth24Stencil8;
		Ref<FramebufferTexture2DAttachment> depthAttach = FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint::DepthStencil, texSpec);

		m_Framebuffer = Framebuffer::Create(spec, { colour0Attach, entityIDAttach }, depthAttach);

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
				if (m_Buffers.size() > 0 && result != m_CurrentConstraint) {
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

				if (valid) {
					m_CurrentConstraint = result;
					m_Buffers.push_back(imageBuffer);
					m_ImageNames.push_back(selectedFile.stem().string());
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
						m_PreviewTexture = TextureImporter::LoadTexture2D(m_Buffers[i], spec);
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
				if (m_PreviewTexture && selected != -1) {
					ImGui::Text(m_ImageNames[selected].c_str());
					ImGui::SameLine();
					ImGui::Text("   (layer: %d)", selected);

					ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
					m_ViewportSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);

					m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
					m_Camera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

					Renderer2D::ResetStats();
					m_Framebuffer->Bind();
					RenderCommand::SetViewport({ 0.0f, 0.0f }, glm::ivec2((glm::ivec2)m_ViewportSize));
					RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
					RenderCommand::Clear();

					Renderer2D::BeginScene(m_Camera, glm::mat4(1.0f));
					Renderer2D::DrawQuad(m_QuadTransform, m_PreviewTexture, glm::vec2(1.0f), glm::vec4(1.0f), -1);
					Renderer2D::EndScene();

					m_Framebuffer->Unbind();

					FramebufferTexture2DAttachment* texAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(m_Framebuffer->GetAttachment(FramebufferAttachmentPoint::Colour0).get());
					uint32_t textureID = texAttachment->GetTexture()->GetRendererID();
					ImGui::Image(textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
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
					m_PreviewTexture = nullptr;
				}

				textureDeleted = -1;
			}
		}
	}
}