#pragma once
#include "Renderer/Resource/OrthographicCamera.h"
#include "Core/Timestep.h"

#include "PaulEngine/Events/MouseEvent.h"
#include "PaulEngine/Events/ApplicationEvent.h"

namespace PaulEngine {
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotationEnabled = false, float moveSpeed = 1.0f, float rotateSpeed = 1.5f);

		const OrthographicCamera& GetCamera() const { return m_Camera; }
		OrthographicCamera& GetCamera() { return m_Camera; }

		void OnUpdate(const Timestep timestep);
		void OnEvent(Event& e);

		void ResizeBounds(float width, float height);

		float m_MoveSpeed;
		float m_RotateSpeed;
	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCamera m_Camera;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f;
		bool m_Rotation;
	};
}