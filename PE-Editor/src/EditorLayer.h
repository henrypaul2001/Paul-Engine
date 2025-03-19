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
		void OnSceneSimulate();
		void OnSceneStop();

		void OnDuplicatedEntity();

		void OnDebugOverlayDraw();

		bool OnKeyUp(KeyReleasedEvent& e);
		bool OnMouseUp(MouseButtonReleasedEvent& e);

		void NewScene();
		void OpenScene();
		void OpenScene(std::filesystem::path filepath);
		void SaveSceneAs(const std::string& filepath = std::string());

		void NewProject();
		bool OpenProject();
		void OpenProject(const std::filesystem::path& path);
		void SaveProjectAs();

		bool CanPickEntities();

		std::string m_CurrentFilepath;

		ShaderLibrary m_ShaderLibrary;
		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;

		Ref<Texture2D> m_IconPlay;
		Ref<Texture2D> m_IconStop;
		Ref<Texture2D> m_IconSimulate;

		glm::vec2 m_ViewportSize;

		bool m_ViewportFocus = false, m_ViewportHovered = false;
		bool m_ShowColliders = true;

		EditorCamera m_Camera;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		Scope<ContentBrowserPanel> m_ContentBrowserPanel;

		int m_GizmoType = -1;

		glm::vec2 m_ViewportBounds[2] = { glm::vec2(0.0f), glm::vec2(0.0f) };

		Entity m_HoveredEntity;

		glm::vec4 m_EntityOutlineColour = glm::vec4(1.0f, 0.5f, 0.8f, 1.0f);
		float m_EntityOutlineThickness = 5.0f;

		enum class SceneState {
			Edit = 0, Play = 1, Simulate = 2
		};
		SceneState m_SceneState = SceneState::Edit;
	};
}