#pragma once
#include <PaulEngine/Asset/Asset.h>
#include <PaulEngine/Renderer/SubTexture2D.h>

namespace PaulEngine
{
	class CreateAtlasWindow
	{
	public:
		CreateAtlasWindow() = default;
		CreateAtlasWindow(AssetHandle baseTexture);

		void SetContext(AssetHandle baseTexture);

		void OnImGuiRender();

		void SetShowWindow(bool showWindow) { m_ShowWindow = showWindow; }
		bool ShouldShowWindow() const { return m_ShowWindow; }
	private:
		AssetHandle m_BaseTexture;
	
		bool m_ShowWindow = false;

		struct SubTextureInput {
			glm::vec2 cellCoords;
			glm::vec2 spriteSize;
		};
		std::unordered_map<std::string, SubTextureInput> m_SubTextureInputMap;
	};
}
