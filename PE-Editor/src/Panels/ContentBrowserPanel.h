#pragma once
#include <filesystem>
#include <map>

#include "PaulEngine/Renderer/Asset/Texture.h"

namespace PaulEngine
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void ImGuiRender();

		float m_ThumbnailSize;

	private:
		void RefreshAssetTree();
		void DrawAssetTree();
		void DrawFileView();

		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
		Ref<Texture2D> m_RefreshIcon;

		struct TreeNode
		{
			std::filesystem::path Path;
			AssetHandle Handle = 0;

			uint32_t Parent = (uint32_t)-1;
			std::map<std::filesystem::path, uint32_t> Children;

			TreeNode(const std::filesystem::path& path, AssetHandle handle) : Path(path), Handle(handle) {}
		};

		std::vector<TreeNode> m_TreeNodes;

		std::map<std::filesystem::path, std::vector<std::filesystem::path>> m_AssetTree;

		enum class BrowserMode
		{
			AssetTree = 0,
			FileSystem = 1
		};

		BrowserMode m_Mode = BrowserMode::AssetTree;
	};
}