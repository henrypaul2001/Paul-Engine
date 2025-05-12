#pragma once
#include "Camera.h"
#include "PaulEngine/Core/Timestep.h"
#include <PaulEngine/Events/Event.h>
#include <PaulEngine/Events/MouseEvent.h>

// Adapted from https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Renderer/EditorCamera.h

namespace PaulEngine
{
	class EditorCamera : public Camera
	{
	public:
		float Gamma = 2.2f;

		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void OnUpdate(const Timestep ts, const bool blockMovement = false);
		void OnEvent(Event& e);

		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		inline void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		virtual const glm::mat4& GetProjection() const override { return m_Projection; }
		virtual float GetGamma() const override { return Gamma; }

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		const glm::vec3& GetPosition() const { return m_Position; }
		glm::quat GetOrientation() const;

		inline float GetPitch() const { return m_Pitch; }
		inline float GetYaw() const { return m_Yaw; }

		inline bool IsMoving() const { return m_IsMoving; }

	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);
		
		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		constexpr float RotationSpeed() { return 0.8f; }
		float ZoomSpeed() const;

		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.01f, m_FarClip = 1000.0f;

		glm::mat4 m_Projection = glm::mat4(1.0f);
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		glm::vec3 m_Position = glm::vec3(0.0f);
		glm::vec3 m_FocalPoint = glm::vec3(0.0f);

		glm::vec2 m_InitialMousePosition = glm::vec2(0.0);

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;

		bool m_IsMoving = false;
		bool m_MovementBlocked = false;
	};
}