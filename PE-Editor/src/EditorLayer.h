#pragma once
#include <PaulEngine.h>
#include <PaulEngine/OrthographicCameraController.h>

namespace PaulEngine {
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(const Timestep timestep) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;

	private:
		bool OnKeyUp(KeyReleasedEvent& e);

		// Temp
		Ref<VertexArray> m_SquareVertexArray;

		ShaderLibrary m_ShaderLibrary;
		OrthographicCameraController m_CameraController;
		Ref<Texture2D> m_Texture, m_Texture2;

		Ref<Texture2D> m_Spritesheet;
		Ref<SubTexture2D> m_TextureStairs, m_TextureBarrel, m_TextureTree, m_TextureInvalid;
		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;

		glm::vec2 m_ViewportSize;

		glm::vec4 m_SquareColour = glm::vec4(0.8f, 0.2f, 0.3f, 1.0f);

		uint32_t m_MapWidth, m_MapHeight;
		std::unordered_map<char, Ref<SubTexture2D>> m_TextureMap;

		bool m_ViewportFocus = false, m_ViewportHovered = false;
	};
}