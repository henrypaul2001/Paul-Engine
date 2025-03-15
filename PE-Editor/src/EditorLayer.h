#pragma once
#include <PaulEngine.h>
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
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
		void OnUIDrawToolbar();
		void OnScenePlay();
		void OnSceneStop();

		bool OnKeyUp(KeyReleasedEvent& e);
		bool OnMouseUp(MouseButtonReleasedEvent& e);

		void NewScene();
		void OpenScene();
		void OpenScene(std::filesystem::path filepath);
		void SaveSceneAs(const std::string& filepath = std::string());

		bool CanPickEntities();

		std::string m_CurrentFilepath;

		ShaderLibrary m_ShaderLibrary;
		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_RuntimeScene;

		Ref<Texture2D> m_IconPlay;
		Ref<Texture2D> m_IconStop;

		glm::vec2 m_ViewportSize;

		bool m_ViewportFocus = false, m_ViewportHovered = false;

		EditorCamera m_Camera;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;

		int m_GizmoType = -1;

		glm::vec2 m_ViewportBounds[2] = { glm::vec2(0.0f), glm::vec2(0.0f) };

		Entity m_HoveredEntity;

		enum class SceneState {
			Edit = 0, Play = 1
		};
		SceneState m_SceneState = SceneState::Edit;
	};
}