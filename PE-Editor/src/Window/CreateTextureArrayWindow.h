#pragma once
#include <PaulEngine/Asset/Asset.h>
#include <PaulEngine/Asset/TextureImporter.h>
#include <PaulEngine/Renderer/Texture.h>
#include <PaulEngine/Renderer/Framebuffer.h>
#include <PaulEngine/Scene/SceneCamera.h>

namespace PaulEngine
{
	class CreateTextureArrayWindow
	{
	public:
		CreateTextureArrayWindow();

		void OnImGuiRender();

		void SetShowWindow(bool showWindow) { m_ShowWindow = showWindow; }
		bool ShouldShowWindow() const { return m_ShowWindow; }

	private:
		bool m_ShowWindow = false;

		std::vector<Buffer> m_Buffers;
		std::vector<std::string> m_ImageNames;
		TextureImporter::ImageFileReadResult m_CurrentConstraint;
		Ref<Texture2D> m_PreviewTexture = nullptr;

		Ref<Framebuffer> m_Framebuffer;
		glm::vec2 m_ViewportSize;
		SceneCamera m_Camera;

		glm::mat4 m_QuadTransform;
	};
}