#pragma once
#include <PaulEngine.h>
#include "Panels/SceneHierarchyPanel.h"

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

		void NewScene();
		void OpenScene();
		void SaveSceneAs(const std::string& filepath = std::string());

		std::string m_CurrentFilepath;

		ShaderLibrary m_ShaderLibrary;

		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;
		Entity m_SquareTwo;
		Entity m_CameraEntity;

		glm::vec2 m_ViewportSize;

		bool m_ViewportFocus = false, m_ViewportHovered = false;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
	};
}