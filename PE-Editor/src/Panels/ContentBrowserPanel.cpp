#include "ContentBrowserPanel.h"

#include "PaulEngine/Project/Project.h"

#include <imgui.h>

namespace PaulEngine
{
	ContentBrowserPanel::ContentBrowserPanel() : m_BaseDirectory(Project::GetAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
	{
		//m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/mingcute--folder-2-fill-light.png");
		//m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/mingcute--file-line-light.png");
		m_DirectoryIcon = Texture2D::Create(TextureSpecification(), Buffer());
		m_FileIcon = Texture2D::Create(TextureSpecification(), Buffer());
	}

	void ContentBrowserPanel::ImGuiRender()
	{
		ImGui::Begin("Content Browser");

		if (m_CurrentDirectory != m_BaseDirectory) {
			ImGui::SameLine();
			if (ImGui::Button("...")) {
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
			ImGui::SameLine();
		}
		ImGui::Text("%s", m_CurrentDirectory.string().c_str());
		ImGui::Separator();

		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1) { columnCount = 1; }
		ImGui::Columns(columnCount, 0, false);

		int id = 0;
		for (auto& p : std::filesystem::directory_iterator(m_CurrentDirectory)) {
			const std::string& filepathString = p.path().string();
			const std::filesystem::path filename = p.path().filename();

			ImGui::PushID(id);
			Ref<Texture2D> icon = p.is_directory() ? m_DirectoryIcon : m_FileIcon;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton("thumbnail", (ImTextureID)icon->GetRendererID(), ImVec2(thumbnailSize, thumbnailSize), ImVec2(0, 1), ImVec2(1, 0));
			ImGui::PopStyleColor();

			if (ImGui::BeginDragDropSource()) {
				const wchar_t* itemPath = p.path().c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				if (p.is_directory()) {
					m_CurrentDirectory /= p.path().filename();
				}
			}
			ImGui::TextWrapped(filename.string().c_str());
			ImGui::NextColumn();

			id++;
			ImGui::PopID();
		}

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);

		ImGui::End();
	}
}