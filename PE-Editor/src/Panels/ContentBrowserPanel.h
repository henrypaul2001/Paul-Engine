#pragma once
#include <filesystem>

#include "PaulEngine/Renderer/Texture.h"

namespace PaulEngine
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void ImGuiRender();

	private:
		std::filesystem::path m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
	};
}