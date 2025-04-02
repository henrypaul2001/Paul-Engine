#pragma once
#include <PaulEngine/Asset/Asset.h>
#include <PaulEngine/Renderer/SubTexture2D.h>
#include <PaulEngine/Renderer/Framebuffer.h>
#include <PaulEngine/Scene/SceneCamera.h>

namespace PaulEngine
{
	class CreateAtlasWindow
	{
	public:
		CreateAtlasWindow(AssetHandle baseTexture);

		void SetContext(AssetHandle baseTexture);

		void OnImGuiRender();

		void SetShowWindow(bool showWindow) { m_ShowWindow = showWindow; }
		bool ShouldShowWindow() const { return m_ShowWindow; }
	private:
		void Init();

		AssetHandle m_BaseTexture;
	
		bool m_ShowWindow = false;

		struct SubTextureInput {
			glm::vec2 cellCoords;
			glm::vec2 spriteSize;
		};
		std::unordered_map<std::string, int> m_NameToInputIDMap;
		std::vector<SubTextureInput> m_SubTextureInputList;
		std::vector<std::string> m_SubTextureNames;

		Ref<Framebuffer> m_Framebuffer;
	
		glm::vec2 m_ViewportSize;
		SceneCamera m_Camera;

		glm::mat4 m_QuadTransform;
	};
}
