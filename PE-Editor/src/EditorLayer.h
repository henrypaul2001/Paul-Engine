#pragma once
#include <PaulEngine.h>

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

		ShaderLibrary m_ShaderLibrary;

		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;
		Entity m_CameraEntity;

		glm::vec2 m_ViewportSize;

		glm::vec4 m_SquareColour = glm::vec4(0.8f, 0.2f, 0.3f, 1.0f);

		bool m_ViewportFocus = false, m_ViewportHovered = false;
	};
}