#include "Camera.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
namespace Engine {
	Camera::Camera(unsigned int scr_width, unsigned int scr_height, glm::vec3 position, glm::vec3 up, float yaw, float pitch, glm::vec3 front, float moveSpeed, float mouseSens, float zoom)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		Front = front;
		MovementSpeed = moveSpeed;
		MouseSensitivity = mouseSens;
		Zoom = zoom;
		NearClip = 0.1f;
		FarClip = 150.0f;
		SCR_WIDTH = scr_width;
		SCR_HEIGHT = scr_height;

		skybox = ResourceManager::GetInstance()->LoadCubemap("Textures/Cubemaps/Space");

		UpdateCameraVectors();
	}

	Camera::~Camera()
	{

	}

	glm::mat4 Camera::GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	
	glm::mat4 Camera::GetProjection()
	{
		return glm::perspective(glm::radians(Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, NearClip, FarClip);
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

		if (constrainPitch)
		{
			if (Pitch > 89.0f) {
				Pitch = 89.0f;
			}
			if (Pitch < -89.0f) {
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