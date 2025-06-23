#include "ContentBrowserPanel.h"

#include "PaulEngine/Project/Project.h"
#include "PaulEngine/Asset/TextureImporter.h"

#include <imgui.h>
#include <PaulEngine/Core/Log.h>

namespace PaulEngine
{
	ContentBrowserPanel::ContentBrowserPanel() : m_BaseDirectory(Project::GetAssetDirectory()), m_CurrentDirectory(m_BaseDirectory), m_ThumbnailSize(128.0f)
	{
		m_TreeNodes.push_back(TreeNode(".", 0));

		m_DirectoryIcon = TextureImporter::LoadTexture2D("Resources/Icons/ContentBrowser/mingcute--folder-2-fill-light.png");
		m_FileIcon = TextureImporter::LoadTexture2D("Resources/Icons/ContentBrowser/mingcute--file-line-light.png");

		RefreshAssetTree();

		m_Mode = BrowserMode::FileSystem;
	}

	void ContentBrowserPanel::ImGuiRender()
	{
		ImGui::Begin("Content Browser");

		const char* label = (m_Mode == BrowserMode::AssetTree) ? "Asset" : "File";
		if (ImGui::Button(label)) {
			m_Mode = (m_Mode == BrowserMode::AssetTree) ? BrowserMode::FileSystem : BrowserMode::AssetTree;
		}

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
		float cellSize = m_ThumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1) { columnCount = 1; }
		ImGui::Columns(columnCount, 0, false);

		if (m_Mode == BrowserMode::AssetTree) {
			DrawAssetTree();
		}
		else if (m_Mode == BrowserMode::FileSystem) {
			DrawFileView();
		}

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &m_ThumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);

		ImGui::End();
	}

	void ContentBrowserPanel::RefreshAssetTree()
	{
		const AssetRegistry& assetRegistry = Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();
		for (const auto& [handle, metadata] : assetRegistry) {
			uint32_t currentNodeIndex = 0;
			
			for (const auto& p : metadata.FilePath) {
				auto it = m_TreeNodes[currentNodeIndex].Children.find(p.string());
				if (it != m_TreeNodes[currentNodeIndex].Children.end()) {
					currentNodeIndex = it->second;
				}
				else {
					TreeNode newNode = TreeNode(p, handle);
					newNode.Parent = currentNodeIndex;
					m_TreeNodes.push_back(newNode);

					m_TreeNodes[currentNodeIndex].Children[p] = m_TreeNodes.size() - 1;
					currentNodeIndex = m_TreeNodes.size() - 1;
				}
			}
		}
	}

	void ContentBrowserPanel::DrawAssetTree()
	{
		TreeNode* node = &m_TreeNodes[0];

		std::filesystem::path currentDirectory = std::filesystem::relative(m_CurrentDirectory, Project::GetAssetDirectory());
		for (const auto& p : currentDirectory) {
			if (node->Path == currentDirectory) {
				break;
			}

			if (node->Children.find(p) != node->Children.end()) {
				node = &m_TreeNodes[node->Children[p]];
				continue;
			}
			else {
				node = nullptr;
				break;
			}
		}

		if (node) {
			for (const auto& [item, treeNodeIndex] : node->Children) {
				bool isDirectory = std::filesystem::is_directory(m_CurrentDirectory / item);
				std::string itemString = item.string();
				if (itemString != "..") {

					ImGui::PushID(itemString.c_str());
					Ref<Texture2D> icon = isDirectory ? m_DirectoryIcon : m_FileIcon;
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
					ImGui::ImageButton("thumbnail", (ImTextureID)icon->GetRendererID(), ImVec2(m_ThumbnailSize, m_ThumbnailSize), ImVec2(0, 1), ImVec2(1, 0));

					if (ImGui::BeginPopupContextItem()) {
						if (ImGui::MenuItem("Delete"))
						{
							PE_CORE_WARN("Asset delete not yet implemented");
						}
						ImGui::EndPopup();
					}

					if (ImGui::BeginDragDropSource()) {
						AssetHandle handle = m_TreeNodes[treeNodeIndex].Handle;
						ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", &handle, sizeof(AssetHandle));
						ImGui::EndDragDropSource();
					}

					ImGui::PopStyleColor();

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						if (isDirectory) {
							m_CurrentDirectory /= item.filename();
						}
					}

					ImGui::TextWrapped(itemString.c_str());

					ImGui::NextColumn();
					ImGui::PopID();
				}
			}
		}
	}

	void ContentBrowserPanel::DrawFileView()
	{
		int id = 0;
		for (auto& p : std::filesystem::directory_iterator(m_CurrentDirectory)) {
			const std::string& filepathString = p.path().string();
			const std::filesystem::path filename = p.path().filename();

			ImGui::PushID(id);
			Ref<Texture2D> icon = p.is_directory() ? m_DirectoryIcon : m_FileIcon;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton("thumbnail", (ImTextureID)icon->GetRendererID(), ImVec2(m_ThumbnailSize, m_ThumbnailSize), ImVec2(0, 1), ImVec2(1, 0));

			if (ImGui::BeginPopupContextItem()) {
				auto relativePath = std::filesystem::relative(p.path(), Project::GetAssetDirectory());
				if (!Project::GetActive()->GetEditorAssetManager()->IsSourceFileRegistered(relativePath)) {
					if (ImGui::MenuItem("Import temporary"))
					{
						Project::GetActive()->GetEditorAssetManager()->ImportAssetFromFile(relativePath, false);
						RefreshAssetTree();
					}
					if (ImGui::MenuItem("Import persistent")) {
						Project::GetActive()->GetEditorAssetManager()->ImportAssetFromFile(relativePath, true);
						RefreshAssetTree();
					}
				}
				ImGui::EndPopup();
			}
			ImGui::PopStyleColor();

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				if (p.is_directory()) {
					m_CurrentDirectory /= p.path().filename();
				}
			}

			if (Project::GetActive()->GetEditorAssetManager()->IsSourceFileRegistered(std::filesystem::relative(p.path(), Project::GetAssetDirectory()))) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
				ImGui::TextWrapped(filename.string().c_str());
				ImGui::PopStyleColor();
			}
			else {
				ImGui::TextWrapped(filename.string().c_str());
			}
			ImGui::NextColumn();

			id++;
			ImGui::PopID();
		}
	}
}