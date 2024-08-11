#pragma once
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
namespace Engine {
	enum CameraMovement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN,
		UP_WORLD,
		DOWN_WORLD
	};
	
	struct ViewPlane {
		glm::vec3 normal;

		// Distance from origin to nearest point in plane
		float distance = 0.0f;

		ViewPlane(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& normal = glm::vec3(0.0f, 1.0f, 0.0f)) {
			this->normal = glm::normalize(normal);
			this->distance = glm::dot(this->normal, position);
		}
	};

	struct ViewFrustum {
		ViewPlane top;
		ViewPlane bottom;

		ViewPlane right;
		ViewPlane left;

		ViewPlane far;
		ViewPlane near;
	};

	class Camera
	{
	public:
		Camera(unsigned int scr_width, unsigned int scr_height, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f, glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f), float moveSpeed = 6.5f, float mouseSens = 0.1f, float zoom = 45.0f);
		~Camera();

		glm::mat4 GetViewMatrix();
		glm::mat4 GetProjection();

		void ProcessKeyboard(CameraMovement direction, float deltaTime);
		void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
		void ProcessMouseScroll(float yoffset);

		// ------------------------ Get ------------------------
		// -----------------------------------------------------
		const glm::vec3& GetPosition() const { return position; }
		const glm::vec3& GetFront() const { return front; }
		const glm::vec3& GetUp() const { return up; }
		const glm::vec3& GetRight() const { return right; }
		const glm::vec3& GetWorldUp() const { return worldUp; }

		float GetYaw() const { return yaw; }
		float GetPitch() const { return pitch; }
		float GetMoveSpeed() const { return movementSpeed; }
		float GetMouseSensitivity() const { return mouseSensitivity; }
		float GetZoom() const { return zoom; }
		float GetNearClip() const { return nearClip; }
		float GetFarClip() const { return farClip; }
		const ViewFrustum& GetViewFrustum() { return viewFrustum; }

		// ------------------------ Set ------------------------
		// -----------------------------------------------------
		void SetPosition(const glm::vec3& newPos) { this->position = newPos; UpdateViewFrustum(); }
		void SetWorldUp(const glm::vec3& newUp) { this->worldUp = newUp; UpdateCameraVectors(); UpdateViewFrustum(); }

		void SetYaw(const float newYaw) { this->yaw = newYaw; UpdateCameraVectors(); UpdateViewFrustum(); }
		void SetPitch(const float newPitch) { this->pitch = newPitch; UpdateCameraVectors(); UpdateViewFrustum(); }
		void SetMoveSpeed(const float newSpeed) { this->movementSpeed = newSpeed; }
		void SetMouseSensitivity(const float newSens) { this->mouseSensitivity = newSens; }
		void SetZoom(const float newZoom) { this->zoom = newZoom; UpdateViewFrustum(); }
		void SetNearClip(const float newClip) { this->nearClip = newClip; UpdateViewFrustum(); }
		void SetFarClip(const float newClip) { this->farClip = newClip; UpdateViewFrustum(); }

	private:
		const ViewFrustum& UpdateViewFrustum();
		void UpdateCameraVectors();

		glm::vec3 position;
		glm::vec3 front;
		glm::vec3 up;
		glm::vec3 right;
		glm::vec3 worldUp;

		float yaw;
		float pitch;

		float movementSpeed;
		float mouseSensitivity;
		float zoom;

		float nearClip;
		float farClip;

		unsigned int SCR_WIDTH;
		unsigned int SCR_HEIGHT;

		ViewFrustum viewFrustum;
	};
}