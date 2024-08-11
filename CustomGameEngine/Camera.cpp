#include "Camera.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
namespace Engine {
	Camera::Camera(unsigned int scr_width, unsigned int scr_height, glm::vec3 position, glm::vec3 up, float yaw, float pitch, glm::vec3 front, float moveSpeed, float mouseSens, float zoom)
	{
		this->position = position;
		this->worldUp = up;
		this->yaw = yaw;
		this->pitch = pitch;
		this->front = front;
		this->movementSpeed = moveSpeed;
		this->mouseSensitivity = mouseSens;
		this->zoom = zoom;
		this->nearClip = 0.1f;
		this->farClip = 500.0f;
		this->SCR_WIDTH = scr_width;
		this->SCR_HEIGHT = scr_height;
		UpdateCameraVectors();
		UpdateViewFrustum();
	}

	Camera::~Camera()
	{

	}

	glm::mat4 Camera::GetViewMatrix()
	{
		return glm::lookAt(position, position + front, up);
	}

	
	glm::mat4 Camera::GetProjection()
	{
		return glm::perspective(glm::radians(zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, nearClip, farClip);
	}
	

	void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
	{
		float velocity = movementSpeed * deltaTime;
		if (direction == FORWARD) {
			position += front * velocity;
		}
		else if (direction == BACKWARD) {
			position -= front * velocity;
		}
		else if (direction == LEFT) {
			position -= right * velocity;
		}
		else if (direction == RIGHT) {
			position += right * velocity;
		}
		else if (direction == UP) {
			position += up * velocity;
		}
		else if (direction == DOWN) {
			position -= up * velocity;
		}
		else if (direction == UP_WORLD) {
			position += worldUp * velocity;
		}
		else if (direction == DOWN_WORLD) {
			position -= worldUp * velocity;
		}

		UpdateViewFrustum();
	}

	void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
	{
		xoffset *= mouseSensitivity;
		yoffset *= mouseSensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (constrainPitch)
		{
			if (pitch > 89.0f) {
				pitch = 89.0f;
			}
			if (pitch < -89.0f) {
				pitch = -89.0f;
			}
		}

		UpdateCameraVectors();
		UpdateViewFrustum();
	}

	void Camera::ProcessMouseScroll(float yoffset)
	{
		zoom -= (float)yoffset;
		if (zoom < 1.0f) {
			zoom = 1.0f;
		}
		else if (zoom > 120.0f) {
			zoom = 120.0f;
		}

		UpdateViewFrustum();
	}

	const ViewFrustum& Camera::UpdateViewFrustum()
	{
		float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
		float fovY = zoom;
		const float halfVSide = farClip * tanf(fovY * 0.5f);
		const float halfHSide = halfVSide * aspect;
		const glm::vec3 frontFar = farClip * front;

		viewFrustum.near = ViewPlane(position + nearClip * front, front);
		viewFrustum.far = ViewPlane(position + frontFar, -front);

		viewFrustum.right = ViewPlane(position, glm::cross(frontFar - right * halfHSide, up));
		viewFrustum.left = ViewPlane(position, glm::cross(up, frontFar + right * halfHSide));

		viewFrustum.top = ViewPlane(position, glm::cross(right, frontFar - up * halfVSide));
		viewFrustum.bottom = ViewPlane(position, glm::cross(frontFar + up * halfVSide, right));

		return viewFrustum;
	}

	void Camera::UpdateCameraVectors()
	{
		glm::vec3 newFront;
		newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		newFront.y = sin(glm::radians(pitch));
		newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(newFront);

		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));
	}
}