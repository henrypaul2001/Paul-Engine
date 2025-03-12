#include "ContentBrowserPanel.h"

#include <imgui.h>

namespace PaulEngine
{
	static const std::filesystem::path s_AssetsDirectory = "assets";

	ContentBrowserPanel::ContentBrowserPanel() : m_CurrentDirectory(s_AssetsDirectory)
	{
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/mingcute--folder-2-fill-light.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/mingcute--file-line-light.png");
	}

	void ContentBrowserPanel::ImGuiRender()
	{
		ImGui::Begin("Content Browser");

		if (m_CurrentDirectory != s_AssetsDirectory) {
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
			const std::filesystem::path filename = p.path().filename();
			ImGui::PushID(id);
			Ref<Texture2D> icon = p.is_directory() ? m_DirectoryIcon : m_FileIcon;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton("thumbnail", (ImTextureID)icon->GetRendererID(), ImVec2(thumbnailSize, thumbnailSize), ImVec2(0, 1), ImVec2(1, 0));
			ImGui::PopStyleColor();
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