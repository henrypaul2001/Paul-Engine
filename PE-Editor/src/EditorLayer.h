#pragma once
#include <PaulEngine.h>
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include <PaulEngine/Renderer/EditorCamera.h>

#include "Window/CreateAtlasWindow.h"
#include "Window/CreateMaterialWindow.h"
#include "Window/CreateTextureArrayWindow.h"

#include "PaulEngine/Renderer/Material.h"

#include "PaulEngine/Renderer/FrameRenderer.h"

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

		void CreateRenderer(FrameRenderer& out_Framerenderer);

		void OnDebugOverlayDraw();

		bool OnKeyUp(KeyReleasedEvent& e);
		bool OnMouseUp(MouseButtonReleasedEvent& e);
		bool OnWindowDrop(WindowDropEvent& e);
		bool OnSceneChanged(SceneChangedEvent& e);
		bool OnViewportResize(MainViewportResizeEvent& e);

		void NewScene();
		void OpenScene();
		void OpenScene(AssetHandle handle);
		void SaveSceneAs(const std::filesystem::path& filepath = std::filesystem::path());

		bool NewProject(std::filesystem::path filepath = std::filesystem::path());
		bool OpenProject();
		bool OpenProject(const std::filesystem::path& path);
		void SaveProjectAs();

		void DrawProjectSelectUI();

		bool CanPickEntities();

		void DrawMaterialEdit(Ref<Material> material);

		std::filesystem::path m_CurrentFilepath;

		FrameRenderer m_Renderer;

		ShaderLibrary m_ShaderLibrary;
		int m_ShadowWidth = 1280;
		int m_ShadowHeight = 1280;
		Ref<Framebuffer> m_MainFramebuffer;

		AssetHandle m_ActiveSceneHandle = 0;
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;

		Ref<Texture2D> m_IconPlay;
		Ref<Texture2D> m_IconStop;
		Ref<Texture2D> m_IconSimulate;

		glm::vec2 m_ViewportSize;

		bool m_ViewportFocus = false, m_ViewportHovered = false;
		bool m_ShowColliders = true;

		Ref<EditorCamera> m_Camera;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		Scope<ContentBrowserPanel> m_ContentBrowserPanel;

		// Windows
		CreateAtlasWindow m_AtlasCreateWindow;
		CreateMaterialWindow m_MaterialCreateWindow;
		CreateTextureArrayWindow m_TextureArrayCreateWindow;

		int m_GizmoType = -1;

		glm::vec2 m_ViewportBounds[2] = { glm::vec2(0.0f), glm::vec2(0.0f) };

		Entity m_HoveredEntity;

		glm::vec4 m_EntityOutlineColour = glm::vec4(1.0f, 0.5f, 0.8f, 1.0f);
		float m_EntityOutlineThickness = 5.0f;

		enum class SceneState {
			Edit = 0, Play = 1, Simulate = 2
		};
		SceneState m_SceneState = SceneState::Edit;

		bool m_ProjectSelected = false;
	};
}