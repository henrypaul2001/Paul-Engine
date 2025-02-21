#include "pepch.h"
#include "OrthographicCameraController.h"

#include "PaulEngine/Core/KeyCodes.h"
#include "PaulEngine/Core/Input.h"

namespace PaulEngine {
	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotationEnabled, float moveSpeed, float rotateSpeed) : m_AspectRatio(aspectRatio), m_Rotation(rotationEnabled), m_MoveSpeed(moveSpeed), 
		m_RotateSpeed(rotateSpeed), m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel)
	{

	}

	void OrthographicCameraController::OnUpdate(const Timestep timestep)
	{
		float moveSpeed = (m_MoveSpeed * m_ZoomLevel) * timestep;
		float rotateSpeed = m_RotateSpeed * timestep;
		if (Input::IsKeyPressed(PE_KEY_W)) {
			m_CameraPosition += glm::vec3(0.0f, moveSpeed, 0.0f);
		}
		if (Input::IsKeyPressed(PE_KEY_S)) {
			m_CameraPosition += glm::vec3(0.0f, -moveSpeed, 0.0f);
		}
		if (Input::IsKeyPressed(PE_KEY_A)) {
			m_CameraPosition += glm::vec3(-moveSpeed, 0.0f, 0.0f);
		}
		if (Input::IsKeyPressed(PE_KEY_D)) {
			m_CameraPosition += glm::vec3(moveSpeed, 0.0f, 0.0f);
		}
		if (m_Rotation && Input::IsKeyPressed(PE_KEY_Q)) {
			m_CameraRotation += 1.0f;
		}
		if (m_Rotation && Input::IsKeyPressed(PE_KEY_E)) {
			m_CameraRotation += -1.0f;
		}

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.DispatchEvent<MouseScrolledEvent>(PE_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.DispatchEvent<WindowResizeEvent>(PE_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset() * 0.5f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}
}