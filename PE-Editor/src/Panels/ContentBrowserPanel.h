#pragma once
#include "PaulEngine/Core/Core.h"
#include <filesystem>

namespace PaulEngine
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void ImGuiRender();

	private:
		std::filesystem::path m_CurrentDirectory;
	};
}