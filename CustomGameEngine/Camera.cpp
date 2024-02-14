#include "Camera.h"
#include <glm/ext/matrix_transform.hpp>
namespace Engine {
	Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, glm::vec3 front, float moveSpeed, float mouseSens, float zoom)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		Front = front;
		MovementSpeed = moveSpeed;
		Zoom = zoom;
		UpdateCameraVectors();
	}

	Camera::~Camera()
	{

	}

	glm::mat4 Camera::GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD) {
			Position += Front * velocity;
		}
		else if (direction == BACKWARD) {
			Position -= Front * velocity;
		}
		else if (direction == LEFT) {
			Position -= Right * velocity;
		}
		else if (direction == RIGHT) {
			Position += Right * velocity;
		}
		else if (direction == UP) {
			Position += Up * velocity;
		}
		else if (direction == DOWN) {
			Position -= Up * velocity;
		}
		else if (direction == UP_WORLD) {
			Position += WorldUp * velocity;
		}
		else if (direction == DOWN_WORLD) {
			Position -= WorldUp * velocity;
		}
	}

	void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		if (constrainPitch) {
			if (Pitch > 89.0f) {
				Pitch = 89.0f;
			}
			else if (Pitch < -89.0f) {
				Pitch = -89.0f;
			}
		}

		UpdateCameraVectors();
	}

	void Camera::ProcessMouseScroll(float yoffset)
	{
		Zoom -= (float)yoffset;
		if (Zoom < 1.0f) {
			Zoom = 1.0f;
		}
		else if (Zoom > 120.0f) {
			Zoom = 120.0f;
		}
	}

	void Camera::UpdateCameraVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);

		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
	}
}