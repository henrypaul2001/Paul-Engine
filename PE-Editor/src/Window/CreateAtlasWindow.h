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
		std::unordered_map<std::string, SubTextureInput> m_SubTextureInputMap;

		Ref<Framebuffer> m_Framebuffer;
	
		glm::vec2 m_ViewportSize;
		//glm::vec2 m_ViewportBounds[2] = { glm::vec2(0.0f), glm::vec2(0.0f) };
		SceneCamera m_Camera;

		glm::mat4 m_QuadTransform;
	};
}
