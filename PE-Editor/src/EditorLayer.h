#pragma once
#include <PaulEngine.h>
#include "Panels/SceneHierarchyPanel.h"
#include <PaulEngine/Renderer/EditorCamera.h>

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
		bool OnMouseUp(MouseButtonReleasedEvent& e);

		void NewScene();
		void OpenScene();
		void SaveSceneAs(const std::string& filepath = std::string());

		std::string m_CurrentFilepath;

		ShaderLibrary m_ShaderLibrary;

		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;

		glm::vec2 m_ViewportSize;

		bool m_ViewportFocus = false, m_ViewportHovered = false;

		EditorCamera m_Camera;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;

		int m_GizmoType = -1;

		glm::vec2 m_ViewportBounds[2] = { glm::vec2(0.0f), glm::vec2(0.0f) };

		int m_HoveredEntityID = -1;
	};
}