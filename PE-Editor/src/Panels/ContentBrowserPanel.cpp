#include "ContentBrowserPanel.h"

#include <imgui.h>

namespace PaulEngine
{
	static const std::filesystem::path s_AssetsDirectory = "assets";

	ContentBrowserPanel::ContentBrowserPanel() : m_CurrentDirectory(s_AssetsDirectory)
	{
	}

	void ContentBrowserPanel::ImGuiRender()
	{
		ImGui::Begin("Content Browser");

		ImGui::Text("%s", m_CurrentDirectory.string().c_str());
		if (m_CurrentDirectory != s_AssetsDirectory) {
			if (ImGui::Button("...")) {
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		for (auto& p : std::filesystem::directory_iterator(m_CurrentDirectory)) {
			const std::filesystem::path filename = p.path().filename();
			if (p.is_directory()) {
				if (ImGui::Button(filename.string().c_str())) {
					m_CurrentDirectory /= p.path().filename();
				}
			}
			else {
				ImGui::Text("%s", filename.string().c_str());
			}
		}

		ImGui::End();
	}
}